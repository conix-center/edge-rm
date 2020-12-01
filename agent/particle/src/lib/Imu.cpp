#include <Particle.h>

#include "lis2dh12.h"
#include "Imu.h"
#include "board.h"
#include "FileLog.h"

void Imu::setup() {
  //set the motion threshold interrupt as an input
  pinMode(ACCEL_INT, INPUT);

  // This code along with the driver was taken from the permamote
  // repo and slightly modified to fit our use case (I2C, latched interrupt)
  accel.config_for_wake_on_motion(200);

  // Clear the interrupt by reading the interrupt status register
  delay(100);
  accel.read_status();
  last_reading = millis();
}

void Imu::update() {
  //every 20 seconds read the accelerometer and shift things around
  if(millis() - last_reading > 20000) {
    moved_40s = moved_20s;
    moved_20s = digitalRead(ACCEL_INT);

    if(moved_20s) {
      moved_since_last_read = 1; 
    }

    accel.read_status();
    last_reading = millis();
  }
}

String Imu::read() {
  // Sample the wake on Interrupt pin
  result = String(moved_since_last_read) + String(MINOR_DLIM) + 
                  String(accel.get_temp()) + String(MINOR_DLIM) +
                  String(digitalRead(ACCEL_INT)) + String(MINOR_DLIM) + 
                  String(moved_20s) + String(MINOR_DLIM) + 
                  String(moved_40s) + String(MINOR_DLIM) +
                  String(accel.get_X()) + String(MINOR_DLIM) + 
                  String(accel.get_Y()) + String(MINOR_DLIM) + 
                  String(accel.get_Z());

  accel.read_status();
  moved_since_last_read = 0;

  return result;
} 
