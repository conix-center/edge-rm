#include <Particle.h>
#include "lib/Gps.h"
#include "lib/FileLog.h"
#include "board.h"

void Gps::powerOn() {
  digitalWrite(GPS_PWR_EN, HIGH);

  delay(100);

  // GPS-specifc setup here
  //t.begin();
  t.gpsOn();
}

void Gps::powerOff() {
  digitalWrite(GPS_PWR_EN, LOW);
}

void Gps::setup() {
  pinMode(GPS_PWR_EN, OUTPUT);

  // GPS-specifc setup here
  t.begin();
  t.gpsOn();
}

void Gps::update() {
    t.updateGPS();
}

String Gps::read() {
  String LatLon = "";
  String GPStime = "";
  String GPSsatellites = "";
  if (t.gpsFix()) {
    LatLon = t.readLatLon();
  } else {
    LatLon = "-1";
  }

  if(t.getTinyGPSPlus()->satellites.value() > 0) {
    struct tm current_time;
    current_time.tm_year = t.getTinyGPSPlus()->date.year() - 1900;
    current_time.tm_mon = t.getTinyGPSPlus()->date.month() - 1;
    current_time.tm_mday = t.getTinyGPSPlus()->date.day();
    current_time.tm_hour = t.getTinyGPSPlus()->time.hour();
    current_time.tm_min = t.getTinyGPSPlus()->time.minute();
    current_time.tm_sec = t.getTinyGPSPlus()->time.second();
    current_time.tm_isdst = 0;

    //convert it into a time_t object
    unsigned long utime = ((unsigned long)mktime(&current_time));
    GPStime = String(utime);
  } else {
    GPStime = "-1";
  }

  GPSsatellites = String(t.getTinyGPSPlus()->satellites.value());
  result = LatLon + String(MINOR_DLIM) + GPStime + String(MINOR_DLIM) + GPSsatellites;
  Serial.printlnf("GPS: %s",result.c_str());
  return result;
}
