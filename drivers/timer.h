#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <types.h>
#include "../arch/i386/isr.h"

#define TIMER_FREQUENCY 100 // 100 Hz = 10 ms per tick

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);
uint32_t timer_get_uptime_seconds(void);
uint32_t timer_get_uptime_ms(void);
void timer_sleep(uint32_t ms);

#endif /* DRIVERS_TIMER_H */
