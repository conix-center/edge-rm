#pragma once

#include <Particle.h>
#include "lis2dh12.h"

class Imu {
  lis2dh12 accel;
  uint8_t moved_40s;
  uint8_t moved_20s;
  uint8_t moved_since_last_read;

  uint32_t last_reading;

  String result;

public:
  void setup();
  void update();
  String read();
};
