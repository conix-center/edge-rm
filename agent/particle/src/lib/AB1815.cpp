#include <lib/AB1815.h>
#include <SPI.h>
#include "time.h"
#include "board.h"

void writeReg(uint8_t reg, uint8_t* buf, uint8_t len) {
     uint8_t start_reg = reg | 0x80;

    //Start the SPI transaction
    SPI.begin(RTC_CS);
    SPI.beginTransaction(__SPISettings(1*MHZ,MSBFIRST,SPI_MODE0));
    digitalWrite(RTC_CS, LOW);
    SPI.transfer(start_reg);

    for(uint8_t i = 0; i < len; i++) {
        SPI.transfer(buf[i]);
    }

    SPI.endTransaction();
    digitalWrite(RTC_CS, HIGH);
    SPI.end();
}

void AB1815::init() {
    uint8_t buf[2];
    buf[0] = 0x9D;
    buf[1] = 0xA5;
    writeReg(AB1815_CFG_KEY_REG, buf, 2);
}

void AB1815::setTime(uint32_t unixTime) {

    struct tm  * time;
    time = gmtime((time_t*)&unixTime);
    uint8_t tx[7];
    tx[0] = 0x00;

    tx[1] = ((time->tm_sec/10 << AB1815_SECOND_TENS_OFFSET) & AB1815_SECOND_TENS_MASK) |
            ((time->tm_sec % 10 << AB1815_SECOND_ONES_OFFSET) & AB1815_SECOND_ONES_MASK);

    tx[2] = ((time->tm_min/10 << AB1815_MINUTE_TENS_OFFSET) & AB1815_MINUTE_TENS_MASK) |
            ((time->tm_min % 10 << AB1815_MINUTE_ONES_OFFSET) & AB1815_MINUTE_ONES_MASK);

    tx[3] = ((time->tm_hour/10 << AB1815_HOUR_TENS_OFFSET) & AB1815_HOUR_TENS_MASK) |
            ((time->tm_hour % 10 << AB1815_HOUR_ONES_OFFSET) & AB1815_HOUR_ONES_MASK);

    tx[4] = ((time->tm_mday/10 << AB1815_DAY_TENS_OFFSET) & AB1815_DAY_TENS_MASK) |
            ((time->tm_mday % 10 << AB1815_DAY_ONES_OFFSET) & AB1815_DAY_ONES_MASK);

    tx[5] = (((time->tm_mon + 1)/10 << AB1815_MON_TENS_OFFSET) & AB1815_MON_TENS_MASK) |
            (((time->tm_mon +1) % 10 << AB1815_MON_ONES_OFFSET) & AB1815_MON_ONES_MASK);

    tx[6] = (((time->tm_year - 100)/10 << AB1815_YEAR_TENS_OFFSET) & AB1815_YEAR_TENS_MASK) |
            (((time->tm_year - 100) % 10 << AB1815_YEAR_ONES_OFFSET) & AB1815_YEAR_ONES_MASK);

    writeReg(AB1815_TIME_DATE_REG, tx, 7);
}

void AB1815::setTimer(uint32_t unixTime) {

    //Write the config to trigger the INT line on an alarm
    //We want OUT2 to not be locked
    //set OSC STATUS to 0x02
    uint8_t val = 0x02;
    writeReg(AB1815_OSC_STATUS, &val, 1);

    //We want OUT2 to be normally high
    //set CTRL1 to 0x33 to make it so that OUT2 is normally high. When it goes
    //low it should trigger a rising edge on WKP if 5v is not applied
    val = 0x33;
    writeReg(AB1815_CTRL_1, &val, 1);

    //We want OUT2 to only change on the nAIRQ
    //set CTRL2 to 0x2C
    val = 0x2C;
    writeReg(AB1815_CTRL_2, &val, 1);

    //Enable the alarm interrupt and make it stay high for 1/4second on match (driving nAIRQ low, triggering circuit)
    //set INTERRUPT_MASK to 0xE4
    val = 0x84;
    writeReg(AB1815_INT_MASK, &val, 1);

    //Set the tiemr control register so that this interrupt occurs once per year
    val = 0x27;
    writeReg(AB1815_TIMER_CTRL, &val, 1);

    //write the alarm time to the alarm registers
    struct tm  * time;
    time = gmtime((time_t*)&unixTime);
    uint8_t tx[7];
    tx[0] = 0x00;

    tx[1] = ((time->tm_sec/10 << AB1815_SECOND_TENS_OFFSET) & AB1815_SECOND_TENS_MASK) |
            ((time->tm_sec % 10 << AB1815_SECOND_ONES_OFFSET) & AB1815_SECOND_ONES_MASK);

    tx[2] = ((time->tm_min/10 << AB1815_MINUTE_TENS_OFFSET) & AB1815_MINUTE_TENS_MASK) |
            ((time->tm_min % 10 << AB1815_MINUTE_ONES_OFFSET) & AB1815_MINUTE_ONES_MASK);

    tx[3] = ((time->tm_hour/10 << AB1815_HOUR_TENS_OFFSET) & AB1815_HOUR_TENS_MASK) |
            ((time->tm_hour % 10 << AB1815_HOUR_ONES_OFFSET) & AB1815_HOUR_ONES_MASK);

    tx[4] = ((time->tm_mday/10 << AB1815_DAY_TENS_OFFSET) & AB1815_DAY_TENS_MASK) |
            ((time->tm_mday % 10 << AB1815_DAY_ONES_OFFSET) & AB1815_DAY_ONES_MASK);

    tx[5] = (((time->tm_mon + 1)/10 << AB1815_MON_TENS_OFFSET) & AB1815_MON_TENS_MASK) |
            (((time->tm_mon +1) % 10 << AB1815_MON_ONES_OFFSET) & AB1815_MON_ONES_MASK);

    tx[6] = (((time->tm_year - 100)/10 << AB1815_YEAR_TENS_OFFSET) & AB1815_YEAR_TENS_MASK) |
            (((time->tm_year - 100) % 10 << AB1815_YEAR_ONES_OFFSET) & AB1815_YEAR_ONES_MASK);

    writeReg(AB1815_ALARM_DATE_REG, tx, 7);

    //clear the status bit so that it can fire
    val = 0x00;
    writeReg(AB1815_STATUS, &val, 1);
}

void AB1815::setTimerFuture(uint32_t seconds) {
    uint32_t time = getTime();
    time += seconds;
    setTimer(time);
}

uint32_t AB1815::getTime(void) {
    uint8_t start_reg = AB1815_TIME_DATE_REG;

    //Start the SPI transaction
    SPI.begin(RTC_CS);
    SPI.beginTransaction(__SPISettings(1*MHZ,MSBFIRST,SPI_MODE0));
    digitalWrite(RTC_CS, LOW);
    SPI.transfer(start_reg);

    struct tm time;
    uint8_t rx[7];

    for(uint8_t i = 0; i < 7; i++) {
        rx[i] = SPI.transfer(0x00);
    }

    SPI.endTransaction();
    digitalWrite(RTC_CS, HIGH);
    SPI.end();
    //digitalWrite(SCK, LOW);
    //digitalWrite(MISO, LOW);
    //digitalWrite(MOSI, LOW);

    time.tm_sec = ((rx[1] & AB1815_SECOND_TENS_MASK) >> AB1815_SECOND_TENS_OFFSET) * 10 +
                    ((rx[1] & AB1815_SECOND_ONES_MASK) >> AB1815_SECOND_ONES_OFFSET);

    time.tm_min = ((rx[2] & AB1815_MINUTE_TENS_MASK) >> AB1815_MINUTE_TENS_OFFSET) * 10 +
                    ((rx[2] & AB1815_MINUTE_ONES_MASK) >> AB1815_MINUTE_ONES_OFFSET);

    time.tm_hour = ((rx[3] & AB1815_HOUR_TENS_MASK) >> AB1815_HOUR_TENS_OFFSET) * 10 +
                    ((rx[3] & AB1815_HOUR_ONES_MASK) >> AB1815_HOUR_ONES_OFFSET);

    time.tm_mday = ((rx[4] & AB1815_DAY_TENS_MASK) >> AB1815_DAY_TENS_OFFSET) * 10 +
                    ((rx[4] & AB1815_DAY_ONES_MASK) >> AB1815_DAY_ONES_OFFSET);

    time.tm_mon = ((rx[5] & AB1815_MON_TENS_MASK) >> AB1815_MON_TENS_OFFSET) * 10 +
                    ((rx[5] & AB1815_MON_ONES_MASK) >> AB1815_MON_ONES_OFFSET) - 1;

    time.tm_year = ((rx[6] & AB1815_YEAR_TENS_MASK) >> AB1815_YEAR_TENS_OFFSET) * 10 +
                    ((rx[6] & AB1815_YEAR_ONES_MASK) >> AB1815_YEAR_ONES_OFFSET) + 100;

    return (uint32_t)mktime(&time);
}
