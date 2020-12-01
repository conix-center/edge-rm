#pragma once
#include <OneWire.h>

class Serialnumber {
  String result;
  uint8_t data[8];
public:
  uint8_t* readBytes();
  String read();
};
