#ifndef DOOR_SENSOR
#define DOOR_SENSOR

#include <Arduino.h>

#define DOOR_SENSOR_PIN   (15)
#define DOORBELL_PIN      (13)

#define DOORBELL_MIN_50HZ_PULSES  (5)       // 5x falling edges = 100ms
#define DOOR_SENSOR_DEBOUNCE_MS   (150)
#define DOORBELL_DEBOUNCE_MS      (2000)    // someone has to be really mad
                                            // to ring more than once in 2s

typedef enum {
    EVENT_NONE = 0x00,
    EVENT_DOOR_OPENED,
    EVENT_DOOR_CLOSED,
    EVENT_HUMAIN_DETECTED,
    EVENT_HUMAIN_DISAPPEARED,
    EVENT_DOORBELL_PRESSED,
    EVENT_DOORBELL_RELEASED     /* RFU? Not used atm */
} sensors_events_e;

typedef struct {
    sensors_events_e type;
    time_t time;
    uint8_t id;
} dated_event_t;

esp_err_t door_init(void);
uint8_t door_sensor_read(void);
uint8_t door_bell_read(void);
void door_task_handle(void);
dated_event_t door_get_event(void);
uint32_t door_count_pending_events(void);
uint8_t door_count_total_events(void);

#endif