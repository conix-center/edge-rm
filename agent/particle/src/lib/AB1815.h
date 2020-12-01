#pragma once

#include <Particle.h>

#define AB1815_TIME_DATE_REG 0x00
#define AB1815_ALARM_DATE_REG 0x08
#define AB1815_STATUS 0x0F
#define AB1815_CTRL_1 0x10
#define AB1815_CTRL_2 0x11
#define AB1815_INT_MASK 0x12
#define AB1815_TIMER_CTRL 0x18
#define AB1815_OSC_STATUS 0x1D
#define AB1815_CFG_KEY_REG 0x1F
#define AB1815_TRICKLE_REG 0x20

#define AB1815_SECOND_TENS_MASK     0x70
#define AB1815_SECOND_TENS_OFFSET   4
#define AB1815_SECOND_ONES_MASK     0x0F
#define AB1815_SECOND_ONES_OFFSET   0

#define AB1815_MINUTE_TENS_MASK     0x70
#define AB1815_MINUTE_TENS_OFFSET   4
#define AB1815_MINUTE_ONES_MASK     0x0F
#define AB1815_MINUTE_ONES_OFFSET   0

#define AB1815_HOUR_TENS_MASK     0x30
#define AB1815_HOUR_TENS_OFFSET   4
#define AB1815_HOUR_ONES_MASK     0x0F
#define AB1815_HOUR_ONES_OFFSET   0

#define AB1815_DAY_TENS_MASK     0x30
#define AB1815_DAY_TENS_OFFSET   4
#define AB1815_DAY_ONES_MASK     0x0F
#define AB1815_DAY_ONES_OFFSET   0

#define AB1815_MON_TENS_MASK     0x10
#define AB1815_MON_TENS_OFFSET   4
#define AB1815_MON_ONES_MASK     0x0F
#define AB1815_MON_ONES_OFFSET   0

#define AB1815_YEAR_TENS_MASK     0xF0
#define AB1815_YEAR_TENS_OFFSET   4
#define AB1815_YEAR_ONES_MASK     0x0F
#define AB1815_YEAR_ONES_OFFSET   0

class AB1815 {
public:
   void     init();
   void     setTime(uint32_t unixTime);
   uint32_t getTime(void);
   void     setTimer(uint32_t unixTime);
   void     setTimerFuture(uint32_t seconds);
};
