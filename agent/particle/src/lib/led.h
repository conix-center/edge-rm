#pragma once

#include "lib/max7315.h"

enum LEDColorState {
    Red,
    Blue,
    Green,
    Purple,
    Orange,
    White,
    Teal,
    Off
};

class led {
public:

    led() {
        io.pinMode(0, OUTPUT);
        io.pinMode(1, OUTPUT);
        io.pinMode(2, OUTPUT);
        //io.digitalWrite(0, HIGH);
        //io.digitalWrite(1, HIGH);
        //io.digitalWrite(2, HIGH);
    };

    const uint8_t GREEN = 2;
    const uint8_t BLUE = 1;
    const uint8_t RED = 0;

    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t intensity;



    void setRed(uint8_t state);
    void setGreen(uint8_t state);
    void setBlue(uint8_t state);
    void setColor(LEDColorState color);
    void setBrightness(uint8_t brightness);
private:
    max7315 io;
};
