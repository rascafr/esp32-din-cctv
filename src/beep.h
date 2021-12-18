#ifndef BEEP_H
#define BEEP_H

#include <Arduino.h>

#define BEEP_PIN  (12)

// /beep 20 50000 = door opened
// /beep 2 200000 = door closed
// /beep 40 15000 = ringbell

#define BEEP_FREQ_HIGH  (15000)
#define BEEP_FREQ_MED   (50000)
#define BEEP_FREQ_LOW   (200000)

void beep_init(void);
void beep_sequence(uint8_t count, uint64_t pres);
void beep_mute(void);
void beep_unmute(void);
bool beep_is_muted(void);

#endif