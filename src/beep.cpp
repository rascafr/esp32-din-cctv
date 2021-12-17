#include "beep.h"

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint8_t current_sequence_count = 0;
volatile uint8_t beep_state = 0;
volatile bool is_muted = false;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  beep_state = !beep_state;
  if (--current_sequence_count == 0) {
    timerAlarmDisable(timer);
    beep_state = 0; // always close task with buzzer off or neighborhood will complain
  }
  digitalWrite(BEEP_PIN, is_muted ? 0 : beep_state); 
  portEXIT_CRITICAL_ISR(&timerMux);
}

void beep_init(void) {
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(BEEP_PIN, LOW);

  // base frequence is 80MHz
  // max prescaler is 65535 (2^16 - 1) = 1220 Hz freq for each timer interrupt
  // use prescaler 80, 1 MHz
  // as a base, counter = 100000 with 1 MHz gives us freq = 10 Hz
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
}

void beep_sequence(uint8_t count, uint64_t pres) {
  portENTER_CRITICAL_ISR(&timerMux);
  current_sequence_count = count << 1; // 1 beep = on + off times
  timerWrite(timer, 0); // reset timer
  timerAlarmWrite(timer, pres, true); // autoreload value, we'll decide when we stop
  timerAlarmEnable(timer);
  timerRestart(timer);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void beep_mute(void) {
  is_muted = true;
}

void beep_unmute(void) {
  is_muted = false;
}
