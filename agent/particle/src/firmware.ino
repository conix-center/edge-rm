// Native
#include <Particle.h>

// Third party libraries
#include <APNHelperRK.h>
#include <CellularHelper.h>
#include <OneWire.h>
#include <simple-coap.h>

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

extern "C" {
  #include "agent_library.h"
  #include "agent_port.h"

}

extern agent_port_timer_cb local_cb;
extern uint8_t timer_flag;

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

//
Coap coap;

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

uint8_t tick = 1;
void tickle() {
  if(tick) {
    digitalWrite(DAC, HIGH);
    tick = 0;
  } else {
    digitalWrite(DAC, LOW);
    tick = 1;
  }
}

Timer wdTimer(1000, tickle);

void setup() {
  //setup the APN credentials
  apnHelper.setCredentials();

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
  pinMode(DAC, OUTPUT);

  gps.setup();

  //Run initial timesync
  timeSync.setup();

  //start watchdog tickle
  wdTimer.start();

  //Setup the particle keepalive
  //The timeout in the Aeris network is 2 minutes
  //But it takes too much data to send a keepalive every 2 minutes
  //So we are just going to ignore that and only contact the device
  //every hour. I will make a script that tries in a loop if I have to
  Particle.keepAlive(60*60);

  //start coap
  coap.start();

  //initialize the agent with the IP address
  agent_init("MY IP");

  //Start the agent library pingrate
  agent_start(360);

  Serial.println("Setup complete.");
}


void loop() {
  // This is the only thing that will happen outside of the state machine!
  // Everything else, including reconnection attempts and cloud update Checks
  // Should happen in the cloud event state
  Particle.process();

  //The software watchdog checks to make sure we are still making it around the loop
  wd.checkin();

  if(timer_flag) {
    timer_flag = 0;
    local_cb();
  }
}
