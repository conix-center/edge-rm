#include <Particle.h>

#include "lib/max7315.h"
#include "lib/led.h"

void led::setRed(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(RED, LOW);
        io.setIndividualIntensity(RED, intensity);
    } else if (state == LOW) {
        io.digitalWrite(RED, HIGH);
        io.setIndividualIntensity(RED, 0x0F);
    }
}

void led::setGreen(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(GREEN, LOW);
        io.setIndividualIntensity(GREEN, intensity);
    } else if (state == LOW) {
        io.digitalWrite(GREEN, HIGH);
        io.setIndividualIntensity(GREEN, 0x0F);
    }
}

void led::setBlue(uint8_t state) {
    if(state == HIGH) {
        io.digitalWrite(BLUE, LOW);
        io.setIndividualIntensity(BLUE, intensity);
    } else if (state == LOW) {
        io.digitalWrite(BLUE, HIGH);
        io.setIndividualIntensity(BLUE, 0x0F);
    }
}

void led::setColor(LEDColorState color) {
    switch(color) {
        case Red:
            red = 1;
            green = 0;
            blue = 0;
        break;
        case Green:
            red = 0;
            green = 1;
            blue = 0;
        break;
        case Purple:
            red = 1;
            green = 0;
            blue = 1;
        break;
        case Blue:
            red = 0;
            green = 0;
            blue = 1;
        break;
        case Orange:
            red = 1;
            green = 1;
            blue = 0;
        break;
        case Teal:
            red = 0;
            green = 1;
            blue = 1;
        break;
        case White:
            red = 1;
            green = 1;
            blue = 1;
        break;
        case Off:
            red = 0;
            green = 0;
            blue = 0;
        break;
    }
    led::setRed(red);
    led::setGreen(green);
    led::setBlue(blue);
}

//only let the user control the bottom four bits of brightness
void led::setBrightness(uint8_t brightness) {
    intensity = brightness;
    led::setRed(red);
    led::setGreen(green);
    led::setBlue(blue);
}

