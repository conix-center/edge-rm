#include "lib/Timesync.h"
#include "Particle.h"

void Timesync::setup() {
  rtc.init();
  timesyncState = unsynced;

  //it seems smart to aggressively rely on the RTC for time sync
  Serial.println("Syncing to RTC");
  uint32_t t = rtc.getTime();
  Serial.printlnf("Setting time to %lu from RTC", t);
  Time.setTime(t);
}

void Timesync::update(CellularState cellularState) {
  //Serial.printlnf("Timesync State: %d", timesyncState);
  switch(timesyncState) {
    case unsynced:
      if(Particle.connected()) {
        //sync time with particle cloud
        Serial.println("Particle connected, trying to sync");
        Particle.syncTime();
        sync_start_time = millis();
        timesyncState = syncingParticle;
      } else if (Cellular.ready() && cellularState != ParticleConnecting) {
        //try to get an update VIA NTP 
        sync_start_time = millis();
        timesyncState = sendNTP;
      } else {
        //we can periodically set particle time to RTC time
      }
    break;
    case syncingParticle:
      if(!Particle.syncTimePending() && Particle.timeSyncedLast() > sync_start_time) {
        last_sync = millis();
        timesyncState = updateRTC;
      } else if (millis() - sync_start_time > 60000) {
        //sync timed out, try to sync via NTP
        sync_start_time = millis();
        timesyncState = sendNTP;
      } else {
        //just let it try to sync
      }
    break;
    case sendNTP:
      //Try to time sync with NTP
      if(udp.begin(2390) != true) {
        //failed - update from RTC
        timesyncState = updateFromRTC;
      } else {
        uint8_t packet[48];
        packet[0] = 0x1B;
        sync_start_time = millis();

        if(udp.sendPacket(packet, 48, IPAddress(128,138,141,172), 123) < 0) {
          udp.stop();
          timesyncState = updateFromRTC;
        }

        timesyncState = waitNTP;
      }
    break;
    case waitNTP: {
      uint8_t packet[48];
      if(udp.receivePacket(packet, 48) > 0) {
        Serial.printlnf("Received udp packet");
        udp.stop();

        unsigned long Receivedmillis = millis();
        if(packet[1] == 0) {
          Serial.println("Received kiss of death.");
          timesyncState = updateFromRTC;
        }
        unsigned long NTPtime = packet[40] << 24 | packet[41] << 16 | packet[42] << 8 | packet[43];
        unsigned long NTPfrac = packet[44] << 24 | packet[45] << 16 | packet[46] << 8 | packet[47];

        if(NTPtime == 0) {
          timesyncState = updateFromRTC;
        }

        unsigned long NTPmillis = (unsigned long)(((double)NTPfrac)  / 0xffffffff * 1000);
        NTPmillis += (Receivedmillis - sync_start_time)/2;
        if(NTPmillis >= 1000) {
          NTPmillis -= 1000;
          NTPtime += 1;
        }

        unsigned long t = NTPtime - 2208988800UL + 1;
        Serial.printlnf("Got time %lu from NTP", t);
        Time.setTime(t);
        last_sync = millis();
        timesyncState = updateRTC;
      } else if (millis() - sync_start_time > 20000) {
        udp.stop();
        timesyncState = updateFromRTC;
      } else  {
        //just wait
      }
      break;
    }
    case updateRTC:
        Serial.printlnf("Synced. Setting RTC time to %d",Time.now());
        rtc.setTime(Time.now());
        timesyncState = synced;
    break;
    case updateFromRTC: {
      //set the time
      Serial.println("Syncing to RTC");
      uint32_t t = rtc.getTime();
      Serial.printlnf("Setting time to %lu from RTC", t);
      Time.setTime(t);
      timesyncState = synced;
      break;
    }
    case synced:
      if ((millis() - last_sync) > Timesync::TWELVE_HOURS || last_sync == 0) {
        timesyncState = unsynced;
      }
    break;
  }
}
