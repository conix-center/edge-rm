// Datasheet: https://datasheets.maximintegrated.com/en/ds/MAX7315.pdf
#pragma once

#define MAX7315_IN 0x00
#define MAX7315_PHASE_0 0x01
#define MAX7315_PORT_CFG 0x03
#define MAX7315_PHASE_1 0x09
#define MAX7315_MASTER 0x0E
#define MAX7315_CFG 0x0F
#define MAX7315_INTENSITY_0_1 0x10
#define MAX7315_INTENSITY_2_3 0x11
#define MAX7315_INTENSITY_4_5 0x12
#define MAX7315_INTENSITY_6_7 0x13

#define MAX7315_I2C_ADDRESS   0x27

class max7315 {
public:
    max7315() {};
    void  pinMode(uint8_t pin, uint8_t mode);
    void  digitalWrite(uint8_t pin, uint8_t direction);
    bool  digitalRead(uint8_t pin);
    void  setGlobalIntensity(uint8_t intensity);
    void  setIndividualIntensity(uint8_t pin, uint8_t intensity);
};
