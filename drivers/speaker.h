#ifndef DRIVERS_SPEAKER_H
#define DRIVERS_SPEAKER_H

#include <types.h>

void speaker_play(uint32_t frequency);
void speaker_stop(void);
void speaker_beep(uint32_t frequency, uint32_t duration_ms);
void speaker_play_melody(void);

#endif /* DRIVERS_SPEAKER_H */
