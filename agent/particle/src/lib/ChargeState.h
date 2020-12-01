#pragma once

#include <Particle.h>

#include "lib/FileLog.h"

/* Checks charging state at `frequency`. Publishes every change. */
class ChargeState {
  String result;

public:
  const String CHARGE_STATE_BATTERY = "b";
  const String CHARGE_STATE_WALL = "w";

  void setup();
  String read();
};
