#pragma once

#include "AssetTrackerRK.h"
#include <Particle.h>

class Gps {

  String result;
  AssetTracker t;

public:
  void setup();
  void powerOn();
  void powerOff();
  void update();
  String read();
};
