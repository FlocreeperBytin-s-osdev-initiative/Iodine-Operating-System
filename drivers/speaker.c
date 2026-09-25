#include "speaker.h"
#include "timer.h"
#include "../arch/i386/io.h"

void speaker_play(uint32_t frequency) {
    if (frequency == 0) return;

    uint32_t div = 1193180 / frequency;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void speaker_stop(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    speaker_play(frequency);
    timer_sleep(duration_ms);
    speaker_stop();
}

void speaker_play_melody(void) {
    // A nice classic 8-bit fanfare melody!
    uint32_t notes[] = { 261, 329, 392, 523, 392, 523 };
    uint32_t durations[] = { 100, 100, 100, 200, 100, 300 };

    for (int i = 0; i < 6; i++) {
        speaker_beep(notes[i], durations[i]);
        timer_sleep(40);
    }
}
