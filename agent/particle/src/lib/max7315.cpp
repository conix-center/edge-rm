#include <Particle.h>

#include <Wire.h>
#include "lib/max7315.h"

void read_reg(uint8_t reg, uint8_t* read_buf, size_t len){
  if (len > 256) return;
  uint8_t readreg = reg;

  if(!Wire.isEnabled()) {
    Wire.begin();
  }

  Wire.beginTransmission(MAX7315_I2C_ADDRESS);
  Wire.write(readreg);
  Wire.endTransmission(false);
  Wire.requestFrom(MAX7315_I2C_ADDRESS, len, true);
  for(size_t i = 0; i < len; i++) {
    read_buf[i] = Wire.read();
  }
}

void write_reg(uint8_t reg, uint8_t* write_buf, size_t len){
  if (len > 256) return;

  Wire.beginTransmission(MAX7315_I2C_ADDRESS);
  Wire.write(reg);
  for(size_t i = 0; i < len; i++) {
    Wire.write(write_buf[i]);
  }
  Wire.endTransmission(true);
}


void max7315::pinMode(uint8_t pin, uint8_t mode) {
 
  uint8_t port_cfg;
  read_reg(MAX7315_PORT_CFG, &port_cfg, 1);

  //now modify the port_cfg
  if(mode == OUTPUT) {
    port_cfg &= ~(0x01 << pin);
  } else {
    port_cfg |= (0x01 << pin);
  }

  write_reg(MAX7315_PORT_CFG, &port_cfg, 1);
}

void max7315::digitalWrite(uint8_t pin, uint8_t direction) {
  uint8_t port_cfg;
  read_reg(MAX7315_PHASE_0, &port_cfg, 1);

  //now modify the port_cfg
  if(direction == LOW) {
    port_cfg &= ~(0x01 << pin);
  } else {
    port_cfg |= (0x01 << pin);
  }

  write_reg(MAX7315_PHASE_0, &port_cfg, 1);
}

bool max7315::digitalRead(uint8_t pin) {
  uint8_t input;
  read_reg(MAX7315_IN, &input, 1);

  return (bool)(input & (0x01 << pin));
}

void max7315::setIndividualIntensity(uint8_t pin, uint8_t intensity) {
  //Set global intensity to FF
  uint8_t temp = 0xFF;
  write_reg(MAX7315_MASTER, &temp, 1);

  //clear the global intensity bit
  uint8_t cfg;
  read_reg(MAX7315_CFG, &cfg, 1);
  cfg &= ~(1 << 2);
  write_reg(MAX7315_CFG, &cfg, 1);

  //calculate the pin to set
  uint8_t reg = MAX7315_INTENSITY_0_1 + pin/2;
  uint8_t offset;
  //calculate the reg offset
  if(pin % 2) {
    offset = 4;
  } else {
    offset = 0;
  }

  intensity = intensity & 0X0F;

  uint8_t current_intensity;
  read_reg(reg, &current_intensity, 1);

  current_intensity &= ~(0x0F << offset);
  current_intensity |= (intensity << offset);
  write_reg(reg, &current_intensity, 1);
}

void max7315::setGlobalIntensity(uint8_t intensity) {
  write_reg(MAX7315_MASTER, &intensity, 1);
}
