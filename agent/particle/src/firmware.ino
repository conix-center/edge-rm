// Native
#include <Particle.h>

// Third party libraries
#include <APNHelperRK.h>
#include <CellularHelper.h>
#include <OneWire.h>

// Our code
#include "board.h"
#include "lib/CellStatus.h"
#include "lib/PowerCheck.h"
#include "lib/AB1815.h"
#include "lib/ChargeState.h"
#include "lib/FileLog.h"
#include "lib/Gps.h"
#include "lib/Imu.h"
#include "lib/SDCard.h"
#include "lib/Timesync.h"
#include "lib/Serialnumber.h"
#include "lib/led.h"
#include "product_id.h"

//***********************************
//* Critical System Config
//***********************************
//Starting with versions greater than 26 this is a SEMANTIC version in two digit decimal
//The first two digits are major revision, we won't use those for now
//The second two digit group is minor revision. We are using this to indicate fundamental hardware changes
//The last revision indicates small changes
//By this scheme we currently have deployed 000026
//Plugwatch F is new firmware with a new data format and would be 000100
//This make s it easier to make minor changes to firmware of different versions

#ifdef PRODUCT
int product_id = PRODUCT;
PRODUCT_ID(PRODUCT);
#else
int product_id = 10804;
PRODUCT_ID(10804);
#endif

int version_int = 212;
PRODUCT_VERSION(212);

SYSTEM_THREAD(ENABLED);
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_MODE(MANUAL);

//***********************************
//* Watchdogs and reset functions
//***********************************
const int HARDWARE_WATCHDOG_TIMEOUT_MS = 100000;
ApplicationWatchdog wd(HARDWARE_WATCHDOG_TIMEOUT_MS, soft_reset);
retained unsigned long reboot_cnt = 0;

int soft_reset_helper(String cmd) {
  soft_reset();
  return 0;
}

void soft_reset() {
  // Soft reset the particle using system sleep
  Serial.println("Resetting by sleeping temporarily");
  System.sleep(SLEEP_MODE_DEEP, 10);
}

int hard_reset(String cmd) {
  // Hard reset the particle using the reset chip
  pinMode(PARTICLE_RST, OUTPUT);
  digitalWrite(PARTICLE_RST, HIGH);
  return 0;
}

//***********************************
//* SD Card
//***********************************
led statusLED;

//***********************************
//* SD Card
//***********************************
SDCard SD;

//***********************************
//* Timesync
//***********************************
Timesync timeSync;
AB1815 rtc;

//***********************************
//* CellStatus
//***********************************
CellStatus cellStatus;

//***********************************
//* PowerCheck
//***********************************
PowerCheck powercheck;

//***********************************
//* Charge state
//***********************************
ChargeState chargeState;

//***********************************
//* IMU
//***********************************
Imu imu; 

//***********************************
//* GPS
//***********************************
Gps gps;

//***********************************
//* Serialnumber
//***********************************
Serialnumber serialNumber;

//***********************************
//* APNs
//***********************************
const APNHelperAPN apns[7] = {
  {"8901260", "wireless.twilio.com"},
  {"8923301", "http://mtnplay.com.gh"},
  {"8991101", "airtelgprs.com"},
  {"8958021", "gprsweb.digitel.ve"},
  {"8958021", "internet.digitel.ve"},
  {"8923400", "9mobile"},
  {"8918500", "iot-eu.aer.net"}
};
APNHelper apnHelper(apns, sizeof(apns)/sizeof(apns[0]));

//***********************************
//* System Data storage
//***********************************

typedef struct {
  String topic;
  String data;
  String dataCRC;
} ParticleMessage;

String serializeParticleMessage(ParticleMessage m) {
  return m.topic + ";" + m.dataCRC + ";" + m.data;
}

ParticleMessage deserializeParticleMessage(String m) {
  int index = m.indexOf(";");
  int second_index = m.indexOf(";",index + 1);
  ParticleMessage p;

  if(index == -1 || second_index == -1 || second_index - index != 5) {
    p.data = "";
    p.topic = "";
    p.dataCRC = "";
  } else {
    p.topic = m.substring(0,index);
    p.dataCRC = m.substring(index+1,index+5);
    p.data = m.substring(index+6);
  }

  return p;
}

String successHash = "";
bool newSuccessResponse = false;
bool newSuccessResponseProcessed = false;
void responseHandler(const char *event, const char *data) {
  Serial.println("Hook Success Response");
  String response(data);
  if(response.indexOf("OK") != -1) {
    String s(event);
    int index = s.lastIndexOf("/");
    if(index > 4 ) {
      successHash = s.substring(index-4,index);  
      Serial.printlnf("Got success response with hash " + successHash);
      newSuccessResponse = true;
      newSuccessResponseProcessed = false;
    } else {
      Serial.printlnf("Got success response with malformed event: %s", event);
    }
  } else {
    Serial.println("Did not get success response");
  }
}

void errorHandler(const char *event, const char *data) {
  Serial.println("Hook Error Response");
  Serial.println(event);
  Serial.println(data);
  Serial.println();
}


// The SD queue is the queue for data t obe sent to the SD cardk
std::queue<ParticleMessage> SDQueue;

// The cloud queue is the queue for data to be send to the cloud
// or put on the data backlog
std::queue<ParticleMessage> CloudQueue;

//The Data log writes data to the SD card
auto DataLog = FileLog(SD, "data_log.txt");
//The data dequeue is the backlog for sending to the cloud
auto DataDequeue = FileLog(SD, "data_dequeue.txt");

void setup() {
  //setup the APN credentials
  apnHelper.setCredentials();

  reboot_cnt++;

  // Set up debugging UART
  Serial.begin(9600);
  Serial.println("Initial Setup.");

  // Set up I2C
  Wire.begin();

  // Setup SD card first so that other setups can log
  SD.setup();

  //setup the other subsystems
  chargeState.setup();
  imu.setup();
  FuelGauge().quickStart();

  //Setup the watchdog toggle pin
  pinMode(WDI, OUTPUT);

  gps.setup();

  //Run initial timesync
  timeSync.setup();

  //Setup the particle keepalive
  //The timeout in the Aeris network is 2 minutes
  //But it takes too much data to send a keepalive every 2 minutes
  //So we are just going to ignore that and only contact the device
  //every hour. I will make a script that tries in a loop if I have to
  Particle.keepAlive(60*60);

  Serial.println("Setup complete.");

  //Register reset functions with the particle cloud
  Particle.function("hard_reset", hard_reset);
  Particle.function("soft_reset", soft_reset_helper);
  Particle.subscribe(System.deviceID() + "/hook-response", responseHandler, MY_DEVICES);
  //We aren't currently using the hook error response
  //Particle.subscribe(System.deviceID() + "/hook-error", errorHandler, MY_DEVICES);
}

//This structure is what all of the drivers will return. It will
//be packetized and send to the cloud in the sendPacket state
#define RESULT_LEN 100
struct ResultStruct {
  char chargeStateResult[RESULT_LEN];
  char mpuResult[RESULT_LEN];
  char wifiResult[RESULT_LEN];
  char cellResult[RESULT_LEN];
  char sdStatusResult[RESULT_LEN];
  char gpsResult[RESULT_LEN];
  char systemStat[RESULT_LEN];
  char SDstat[RESULT_LEN];
};

// A function to clear all the fields of a resultStruct
void clearResults(ResultStruct* r) {
  r->chargeStateResult[0] = 0;
  r->mpuResult[0] = 0;
  r->wifiResult[0] = 0;
  r->cellResult[0] = 0;
  r->sdStatusResult[0] = 0;
  r->gpsResult[0] = 0;
  r->systemStat[0] = 0;
  r->SDstat[0] = 0;
}

// A function to take all of the resutl strings and concatenate them together
String stringifyResults(ResultStruct r) {
  String result = "";
  result += String(Time.now());
  result += MINOR_DLIM;
  result += String(millis());
  result += MAJOR_DLIM;
  result += String(r.chargeStateResult);
  result += MAJOR_DLIM;
  result += String(r.mpuResult);
  result += MAJOR_DLIM;
  result += String(r.wifiResult);
  result += MAJOR_DLIM;
  result += String(r.cellResult);
  result += MAJOR_DLIM;
  result += String(r.sdStatusResult);
  result += MAJOR_DLIM;
  result += String(r.gpsResult);
  result += MAJOR_DLIM;
  result += String(r.systemStat);
  result += MAJOR_DLIM;
  result += String(r.SDstat);
  return result;
}

// retain this so that on the next iteration we still get results on hang
ResultStruct sensingResults;

/* LOOP State enums and their global delcarations*/
// When we wake up we always want to start in sleep 
// Because we might just want to tickle the watchdog and sleep again
SystemState state = Sleep;

PowerState powerState = Unknown;

CellularState cellularState = InitiateParticleConnection;

WatchdogState watchdogState = WatchdogLow;

CollectionState collectionState = WaitForCollection;

SleepState sleepState = SleepToAwakeCheck;

SendState sendState = ReadyToSend;

LEDFlashingState ledFlashingState = Solid;
LEDColorState ledColorState = Teal;
/* END Loop delcaration variables*/

/* Configration variables and times*/

//This helps us calculate when the next collection interval is
retained uint32_t last_collection_time;

//These are flags for the watchdog. It is only tickled when these are true.
//They are changed to true and false depending on success or failure of tried events
bool sendSuccess = true;
bool logSuccess = true;
bool service = false;

//This sets a timer that keeps us from sleeping for a set period of time
uint32_t power_off_millis;

//2 minutes
const uint32_t  COLLECTION_INTERVAL_SECONDS = 2 * 60;

//2 minutes
const uint32_t SEND_BACKOFF_MAX_MS = 2 * 60 * 1000;

//four hours
const uint32_t  SLEEP_COLLECTION_INTERVAL_SECONDS = 4 * 60 * 60;
/* END Configration variables and times*/

/* Variables that track whether or not we should use the watchdog */

//#define PRINT_STATE
void printState(String name, int state) {
  #ifdef PRINTSTATES
  Serial.printlnf("%s: %d",name.c_str(),state);
  #endif
}


void loop() {
  // This is the only thing that will happen outside of the state machine!
  // Everything else, including reconnection attempts and cloud update Checks
  // Should happen in the cloud event state
  Particle.process();

  //The software watchdog checks to make sure we are still making it around the loop
  wd.checkin();

  printState("state", state);
  switch(state) {

    /*Initializes the state machine and board. Decides when to sleep or wake*/
    case Sleep: {
      //Check if you should go to sleep. Manage waking up and sending periodic check ins
      printState("sleep state", sleepState);
      switch(sleepState) {
        case SleepToAwakeCheck:
          // We are here either because 
          //  1) the program just started
          //        - If this is the case we are either powered or past our sleep collection interval
          //  2) the RTC woke us up to collect and send data 
          //        - this should be true if we are past our sleep collection interval
          //  3) a power restoration woke us up and we now should be awake and send data
          //        - If this is the case the power will be on
          //  4) The particle woke us and we should just tickle the watchdog and go back to sleep
          //        - This would be true if none of the above are true.

          if(/*powercheck.getHasPower()*/true) {
            //We should just be awake
            state = Sleep;
            sleepState = PrepareForWake;
            Serial.println("Transitioning to PrepareForWake");
          } else if (last_collection_time/SLEEP_COLLECTION_INTERVAL_SECONDS != Time.now()/SLEEP_COLLECTION_INTERVAL_SECONDS) {
            //have we rolled over to a new collection interval? truncating division shoul handle the modulus of the interval time.
            state = Sleep;
            sleepState = PrepareForWake;
          } else {
            //We just woke up to tickle the watchdog and sleep again
            state = Sleep;
            //sleepState = PrepareForSleep;
          }
        break;
        case PrepareForWake:
          //In this state we turn everything on then let the state machine progress
          // Turn on the GPS
          Serial.println("Turning GPS on");
          gps.powerOn();

          // Turn on the STM and voltage sensing
          //Serial.println("Turning AC sense on");
          //digitalWrite(AC_PWR_EN, HIGH);

          // Turn on the SD card
          Serial.println("Turning SD card on");
          SD.PowerOn();

          //Proceed through both state machines
          Serial.println("Transitioning to AwakeToSleepCheck");
          sleepState = AwakeToSleepCheck;
          state = CheckPowerState;
        break;
        case AwakeToSleepCheck: {
          static bool power_check_once = false;
          //We should not sleep if
          // - we have power
          // - we are currently collecting data
          // - Data has just been collected and needs to be sent/queued
          sleepState = AwakeToSleepCheck;
          state = CheckPowerState;
          /*if(powercheck.getHasPower()) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
            power_check_once = false;
          } else if (collectionState != WaitForCollection || cellularState == ParticleConnecting || cellularState == InitiateParticleConnection) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else if (CloudQueue.size() != 0 || SDQueue.size() != 0) {
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else if (millis() - power_off_millis < 62000 || power_check_once == false) {
            //we should wait a bit after the power goes off.
            //we don't want to sleep for random power transients
            power_check_once = true;
            sleepState = AwakeToSleepCheck;
            state = CheckPowerState;
          } else {
            sleepState = PrepareForSleep;
            Serial.println("Transitioning to PrepareForSleep");
            state = Sleep;
          }*/
          break;
        }
        case PrepareForSleep:
          //In this state we turn everything off then go to sleep. Using sleep mode deep starts
          //Everything back from the beginning
          // Turn off the GPS
          digitalWrite(GPS_PWR_EN, LOW);
          // Turn off the STM and voltage sensing
          digitalWrite(AC_PWR_EN, LOW);
          // Turn off the SD card
          SD.PowerOff();

          // Toggle the watchdog to make sure that it doesn't trigger early
          digitalWrite(WDI, HIGH);
          delay(100);
          digitalWrite(WDI, LOW);

          //make sure the rtc interrupt is clear so that we wake up
          //we can do this by reinitializing the RTC
          timeSync.setup();
        
          // Set two timers
          // One timer on the RTC for the next time we want to collect data
          // Another timer for the particle so that we can tickle the watchdog
          
          //calculate the next time we want to collect data
          // Get the current time
          uint32_t current_time = Time.now();
          // the division truncates this time to the last time we would have collected
          uint32_t collection_number = current_time/SLEEP_COLLECTION_INTERVAL_SECONDS;
          // calculate the next time we should collect
          uint32_t new_collection_number = collection_number + 1;
          uint32_t new_collection_time = new_collection_number*SLEEP_COLLECTION_INTERVAL_SECONDS;
          //Serial.printlnf("Current time: %d, waking up at %d for next collection.",current_time, new_collection_time);
          rtc.setTimer(new_collection_time);

          //set the LED to red
          ledColorState = Red;
          statusLED.setBrightness(5);
          //statusLED.setColor(ledColorState);
          statusLED.setColor(Off);

          //Cellular.off();
          pinMode(PWR_UC, OUTPUT);
          digitalWrite(PWR_UC, LOW);
          delay(1500);
          digitalWrite(PWR_UC, HIGH);

          delay(2000);

          Serial.println("Going to sleep");
          Serial.flush();
          Serial.end();

          // Now go to sleep with the system sleep set to wakeup for the watchdog
          System.sleep(SLEEP_MODE_DEEP, 600);

          //If for some reason this fails try again?
          state = Sleep;
          //sleepState = PrepareForSleep;
        break;
      }
    break;
    }

    /*Checks for power state changes and generates events for them*/
    case CheckPowerState: {
      switch(powerState) {
        case Unknown:
          if(powercheck.getHasPower()) {
             powerState = Powered;
          } else {
             powerState = Unpowered;
             power_off_millis = millis();
          }
        break;
        case Powered:
          if(!powercheck.getHasPower()) {
             powerState = Unpowered;
             power_off_millis = millis();
          } 
        break;
        case Unpowered:
          if(powercheck.getHasPower()) {
             powerState = Powered;
          } 
        break;
      }
      state = MaintainCellularConnection;    
    break;
    }

    /*Manages connection to the particle cloud or cellular network*/
    case MaintainCellularConnection: {
      printState("cellular state", cellularState);
      static unsigned long connection_start_time;
      switch(cellularState) {
        case InitiateParticleConnection: 
          if(!Particle.connected()) {
            Particle.connect();
            connection_start_time = millis();
            cellularState = ParticleConnecting;
          } else {
            cellularState = ParticleConnected;
          }
        break;
        case ParticleConnecting:
          if(Particle.connected()) {
            cellularState = ParticleConnected;
          } else if(millis() - connection_start_time > 600000) {
            //try to connect to cellular network as a backup to get time
            //stop trying to connect to particle
            Particle.disconnect();
            cellularState = InitiateParticleConnection;
          } else {
            //do nothing - let it try to connect
          }
        break;
        case ParticleConnected:
          if(!Particle.connected()) {
            cellularState = InitiateParticleConnection;
          }
        break;
      }
      state = CheckTimeSync;
      break;
    }
    /*Keeps the device time up to date*/
    case CheckTimeSync: {
      timeSync.update(cellularState);
      state = LogData;
      break;
    }

    /*Saves data to the SD card*/
    case LogData: {
      //just turn it on for good measure, although we should just leave it on
      SD.PowerOn();
      if(SDQueue.size() > 0) {
        if(DataLog.appendAndRotate(serializeParticleMessage(SDQueue.front()), Time.now())) {
          //Error writing data to the SD card - do something with it
          Serial.println("Error writing data to the SD card");
        } else {
          logSuccess = true;
          SDQueue.pop();
        }
      }

      state = SendData;
      break;
    }

    /*Sends data to the cloud either from the event queue or from the SD card based backlog*/
    case SendData: {
      static uint32_t last_send_time;
      static uint32_t send_backoff_time = 5000;
      static String last_sent_from = "";
      switch(sendState) {
        case ReadyToSend:
          SD.PowerOn();
          if(CloudQueue.size() > 0) {
            if(Particle.connected()) {
                last_send_time = millis();
                sendState = SendPaused;
                //generate the topic as a unique hash of the data
                if(!Particle.publish(CloudQueue.front().topic + "/" + CloudQueue.front().dataCRC,CloudQueue.front().data, PRIVATE)) {
                  //should handle this error
                  Serial.println("Failed to send packet. Appending to dequeue.");
                  if(DataDequeue.append(serializeParticleMessage(CloudQueue.front()))) {
                    //should handle this error
                    Serial.println("Failed to append to dequeue");
                  } else {
                    Serial.println("Appended to dequeue successfully");
                    CloudQueue.pop();
                  }
                } else {
                  last_sent_from = "CloudQueue";
                  Serial.println("Sent message with CRC " + CloudQueue.front().dataCRC + " to particle cloud - waiting on webhook response");
                  newSuccessResponse = false;
                  //CloudQueue.pop();
                }
            } else {
              if(DataDequeue.append(serializeParticleMessage(CloudQueue.front()))) {
                //should handle this error
                Serial.println("Failed to append to dequeue");
              } else {
                Serial.println("Appended to dequeue successfully");
                logSuccess = true;
                CloudQueue.pop();
              }
            }
          } else if(DataDequeue.getFileSize() > 0) {
            //Serial.printlnf("DataDequeue file size: %d", DataDequeue.getFileSize());
            if(Particle.connected()) {
              ParticleMessage m = deserializeParticleMessage(DataDequeue.getLastLine());
              last_send_time = millis();
              sendState = SendPaused;
              if(m.topic == "") {
                //This is an ill formatted message in the datadequeue(possible from an old version. Remove it
                Serial.println("Removing bad message from data dequeue");
                DataDequeue.removeLastLine();
              } else if(!Particle.publish(m.topic + "/" + m.dataCRC, m.data, PRIVATE)) {
                //should handle this error
                Serial.println("Failed to send from dequeue");
              } else {
                last_sent_from = "DataDequeue";
                Serial.println("Sent message from dequeue with CRC " + m.dataCRC + " to particle cloud - waiting on webhook response");
                //DataDequeue.removeLastLine();
                newSuccessResponse = false;
              }
            }
          } else if(DataDequeue.getFileSize() == -1) {
          }

          state = CollectPeriodicInformation;
          break;
        case SendPaused:
          if(newSuccessResponse == true && newSuccessResponseProcessed == false) {
            
            Serial.println("Processing success response");

            // clear the new response
            newSuccessResponseProcessed = true;

            // process the response
            if(last_sent_from == "CloudQueue") {
              if(successHash == CloudQueue.front().dataCRC && successHash != "") {
                //great it succeeded
                Serial.println("Webhook success CRC " + successHash + " matched - popping from queue");
                CloudQueue.pop();
                send_backoff_time = 5000;
                Serial.println("Send backoff time reset");
              } else {
                //it did not succeeded - put it on the dequeue
                Serial.println("Webhook success hash " + successHash + " did not match last sent CRC " + CloudQueue.front().dataCRC + ". Moving to data to dequeue");

                if(send_backoff_time < SEND_BACKOFF_MAX_MS) {
                  send_backoff_time = send_backoff_time * 2;
                  if(send_backoff_time > SEND_BACKOFF_MAX_MS) {
                    send_backoff_time = SEND_BACKOFF_MAX_MS; 
                  }
                }
                Serial.printlnf("Send backoff time now %d", send_backoff_time/1000);


                Serial.println("Clearing cloud queue");
                while(CloudQueue.size() != 0) {
                  if(DataDequeue.append(serializeParticleMessage(CloudQueue.front()))) {
                    //should handle this error
                    Serial.println("Failed to append to dequeue");

                    //break from the loop and leave them on the Cloud Queue
                    //We don't want to loop forever
                    break;
                  } else {
                    Serial.println("Appended to dequeue successfully");
                    CloudQueue.pop();
                  }
                }
              }
            } else if (last_sent_from == "DataDequeue") {
              ParticleMessage m = deserializeParticleMessage(DataDequeue.getLastLine());
              if(successHash == m.dataCRC && successHash != "") {
                //great it succeeded
                Serial.println("Webhook success hash " + successHash + " matched - removing from dequeue");
                DataDequeue.removeLastLine();
                sendSuccess = true;
                send_backoff_time = 5000;
                Serial.println("Send backoff time reset");
              } else {
                //it did not succeeded - just leave it in the dequeue
                Serial.println("Webhook success hash " + successHash + " did not match last sent CRC " + m.dataCRC + ". Leaving on Dequeue");

                if(send_backoff_time < SEND_BACKOFF_MAX_MS) {
                  send_backoff_time = send_backoff_time * 2;
                  if(send_backoff_time > SEND_BACKOFF_MAX_MS) {
                    send_backoff_time = SEND_BACKOFF_MAX_MS;
                  }
                }


                Serial.printlnf("Send backoff time now %d", send_backoff_time/1000);
              }
            } else {
              //Maybe we haven't sent anything yet - just leave it
            }
          } 

          if(millis() - last_send_time > send_backoff_time) {
            if(newSuccessResponse && newSuccessResponseProcessed) {
              //We already cleared the messages - just transition to ready to send
            } else {
              //we didn't get and process success response
              if(last_sent_from == "CloudQueue") {
                  //We didn't receive a response in time - put it in the dequeue
                  Serial.println("Did not receive webhook response in time. Moving to data to dequeue");

                  if(send_backoff_time < SEND_BACKOFF_MAX_MS) {
                    send_backoff_time = send_backoff_time * 2;
                    if(send_backoff_time > SEND_BACKOFF_MAX_MS) {
                      send_backoff_time = SEND_BACKOFF_MAX_MS;
                    }
                  }


                  Serial.printlnf("Send backoff time now %d", send_backoff_time/1000);

                  //In a loop empty the cloud Queue to the DataDequeue
                  while(CloudQueue.size() != 0) {
                    if(DataDequeue.append(serializeParticleMessage(CloudQueue.front()))) {
                      //should handle this error
                      Serial.println("Failed to append to dequeue");

                      //break from the loop and leave them on the Cloud Queue
                      //We don't want to loop forever
                      break;

                    } else {
                      Serial.println("Appended to dequeue successfully");
                      CloudQueue.pop();
                    }
                  }
              } else if (last_sent_from == "DataDequeue") {
                  //it did not succeeded - just leave it in the dequeue
                  Serial.println("Did not receive webhook response in time. Leaving data on dequeue");

                  if(send_backoff_time < SEND_BACKOFF_MAX_MS) {
                    send_backoff_time = send_backoff_time * 2;
                    if(send_backoff_time > SEND_BACKOFF_MAX_MS) {
                      send_backoff_time = SEND_BACKOFF_MAX_MS;
                    }
                  }
                  

                  Serial.printlnf("Send backoff time now %d", send_backoff_time/1000);
              } else {
                //Maybe we haven't sent anything yet - just leave it
              }
            }
            
            sendState = ReadyToSend;
            Serial.println();
          }

          state = CollectPeriodicInformation;
        break;
      }
      break;
    }

    /*Periodically collects summary metrics from the device and generates those events*/
    case CollectPeriodicInformation: {
      printState("collection state", collectionState);
      switch(collectionState) {
        case WaitForCollection: {
          //Decide if we need to collect
          uint32_t current_time = Time.now();
          uint32_t last_collection_number = last_collection_time/COLLECTION_INTERVAL_SECONDS;
          uint32_t current_collection_number = current_time/COLLECTION_INTERVAL_SECONDS;
          //Serial.printlnf("Current time: %d, Last collection time: %d",current_time, last_collection_time);
          if(last_collection_number != current_collection_number) {
            collectionState = CollectResults;
          }
          
          //Also service the things that need to be serviced when we aren't
          //collectinga
          gps.update();
          imu.update();
          break;
        }
        case CollectResults: {
          Serial.println("Collecting");
          strncpy(sensingResults.mpuResult,imu.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.gpsResult,gps.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.sdStatusResult,SD.getResult().c_str(), RESULT_LEN);
          strncpy(sensingResults.chargeStateResult,chargeState.read().c_str(), RESULT_LEN);
          strncpy(sensingResults.cellResult,cellStatus.read().c_str(), RESULT_LEN);
          snprintf(sensingResults.systemStat, RESULT_LEN, "%lu|%s|%u", 0, serialNumber.read().c_str(),reboot_cnt);
          snprintf(sensingResults.SDstat,RESULT_LEN, "%u|%d|%d",0,DataLog.getRotatedFileSize(Time.now()),DataDequeue.getFileSize());
          String result = stringifyResults(sensingResults);
          ParticleMessage m;
          m.topic = "g";
          m.data = result;
          char crc[5];
          snprintf(crc,5,"%04X",OneWire::crc16((uint8_t*)result.c_str(),result.length()));
          String crcString(crc);
          m.dataCRC = crcString;
          SDQueue.push(m);
          CloudQueue.push(m);
          last_collection_time = Time.now();
          collectionState = WaitForCollection;
          break;
        }
      }
      state = ServiceWatchdog;
    break;
    }

    /*manages the watchdog and light on the device*/
    case ServiceWatchdog: {
      static unsigned int lastWatchdog;
      //Service the watchdog if
      //we are getting log failures
      //we are getting send failures
      //we are not connecting to the cloud
      if(cellularState == ParticleConnected && sendSuccess && logSuccess) {
        service = true;
        sendSuccess = false;
        logSuccess = false;
        Serial.println("Servicing watchdog now - both variables are true");
      } 

      switch(watchdogState) {
        case WatchdogHigh:
          digitalWrite(DAC, HIGH);
          if(millis() - lastWatchdog > 1000) {
            lastWatchdog = millis();
            watchdogState = WatchdogLow;
          }
        break;
        case WatchdogLow:
          digitalWrite(DAC, LOW);
          if(millis() - lastWatchdog > 1000 && service) {
            lastWatchdog = millis();
            watchdogState = WatchdogHigh;
            service = false;
          } 
        break;
      }
      state = ServiceLED;
    break;
    }

    case ServiceLED: {
      static uint8_t current_led_brightness = 6;
      static bool current_led_state = true;
      static uint32_t last_switch_time = 0;
      switch(ledFlashingState) {
        case Solid:
          //do nothing
        break;
        case Blinking:
          if(millis() - last_switch_time > 150) {
            if(current_led_state) {
              statusLED.setColor(Off);
              last_switch_time = millis();
              current_led_state = false;
            } else {
              statusLED.setBrightness(2);
              statusLED.setColor(ledColorState);
              last_switch_time = millis();
              current_led_state = true;
            }
          }
        break;
        case Breathing:
          if(millis() - last_switch_time > 100) {
            last_switch_time = millis();
            if(current_led_state) {
              current_led_brightness--;
              statusLED.setBrightness(current_led_brightness);
              statusLED.setColor(ledColorState);
              if(current_led_brightness == 0) {
                current_led_state = false;
              }
            } else {
              current_led_brightness++;
              statusLED.setBrightness(current_led_brightness);
              statusLED.setColor(ledColorState);
              if(current_led_brightness >= 6) {
                current_led_state = true;
                current_led_brightness = 6;
              }
            }
          }
        break;
      }

      if(powercheck.getHasPower()) {
        if(cellularState == ParticleConnecting) {
          ledFlashingState = Blinking;
          ledColorState = Teal;
        } else if (cellularState == ParticleConnected) {
          ledFlashingState = Breathing;
          ledColorState = Teal;
        } else {
          ledFlashingState = Breathing;
          ledColorState = Blue;
        }
      } else {
        if(cellularState == ParticleConnecting) {
          ledFlashingState = Blinking;
          ledColorState = Orange;
        } else if (cellularState == ParticleConnected) {
          ledFlashingState = Breathing;
          ledColorState = Orange;
        } else {
          ledFlashingState = Breathing;
          ledColorState = Red;
        }
      }

      state = Sleep;
      break;
    }
    
    default: {
      state = Sleep;
      break;
    }
  }


}


