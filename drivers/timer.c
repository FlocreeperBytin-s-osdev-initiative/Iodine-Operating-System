#include "timer.h"
#include "../arch/i386/io.h"
#include "../arch/i386/isr.h"

static volatile uint32_t timer_ticks = 0;
static uint32_t timer_freq = 100;

extern void sched_timer_tick(registers_t *regs);

static void timer_callback(registers_t *regs) {
    timer_ticks++;
    sched_timer_tick(regs);
}

void timer_init(uint32_t frequency) {
    timer_freq = frequency ? frequency : 100;
    register_interrupt_handler(32, timer_callback); // IRQ 0 = 32

    uint32_t divisor = 1193180 / timer_freq;

    // Send command byte (channel 0, lobyte/hibyte, rate generator)
    outb(0x43, 0x36);

    // Divisor must be sent byte by byte
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t timer_get_uptime_seconds(void) {
    return timer_ticks / timer_freq;
}

uint32_t timer_get_uptime_ms(void) {
    return (timer_ticks * 1000) / timer_freq;
}

void timer_sleep(uint32_t ms) {
    uint32_t ticks_to_wait = (ms * timer_freq) / 1000;
    if (ticks_to_wait == 0 && ms > 0) ticks_to_wait = 1;
    uint32_t target_ticks = timer_ticks + ticks_to_wait;

    while (timer_ticks < target_ticks) {
        hlt();
    }
}
