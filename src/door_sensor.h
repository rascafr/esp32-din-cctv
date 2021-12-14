#ifndef DOOR_SENSOR
#define DOOR_SENSOR

#include <Arduino.h>

#define DOOR_SENSOR_PIN   (12)
#define DOOR_SENSOR_DEBOUNCE_MS   (150)

typedef enum {
    EVENT_NONE = 0x00,
    EVENT_DOOR_OPENED,
    EVENT_DOOR_CLOSED,
    EVENT_HUMAIN_DETECTED,
    EVENT_HUMAIN_DISAPPEARED
} sensors_events_e;

typedef struct {
    sensors_events_e type;
    time_t time;
    uint8_t id;
} dated_event_t;

esp_err_t door_sensor_init(void);
uint8_t door_sensor_read(void);
void door_sensor_task_handle(void);
dated_event_t door_sensor_get_event(void);
uint32_t door_sensor_get_pending_events_count(void);

#endif