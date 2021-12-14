#include "door_sensor.h"
#include "utils.h"

/*
 * Mutexes
 */
portMUX_TYPE doorSensorMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE PIRSensorMux = portMUX_INITIALIZER_UNLOCKED;

/*
 * ISR variables 
 */
volatile uint32_t doorSensorLastISR = 0;
volatile bool doorSensorLastState = true;
volatile bool doorSensorISRTriggered = false;

// private variables
bool saveTriggered;
bool saveLastState;
uint32_t saveDebounceTimeout;

/*
 * Quick and dirty FIFO for event lists
 */
#define MAX_EVENTS_COUNT 10
dated_event_t events_list[MAX_EVENTS_COUNT];
dated_event_t * ptr_curr_event = NULL;
uint32_t count_pending_events = 0; // cannot be > MAX_EVENTS_COUNT
//sensors_events_e last_queued_event = EVENT_NONE;
static uint8_t __id = 0;

void push_event_to_queue(sensors_events_e new_event) {
  if (count_pending_events > 0) {
    for (uint8_t i=count_pending_events;i>0;i--) {
      memcpy(&events_list[i], &events_list[i-1], sizeof(dated_event_t));
    }
  }
  events_list[0] = { .type = new_event, .time = utils_get_time(), .id = __id++ };
  if (count_pending_events < MAX_EVENTS_COUNT) count_pending_events++;
  last_queued_event = new_event;
}

dated_event_t * pop_event_from_queue(void) {
  if (count_pending_events > 0) {
    count_pending_events--;
    return &events_list[count_pending_events];
  } else return NULL;
}

/*
 * Private local methods
 */
void IRAM_ATTR ISR_DoorSensor_Opened(void * context);

esp_err_t door_sensor_init(void) {
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  esp_err_t e1 = gpio_isr_handler_add((gpio_num_t)DOOR_SENSOR_PIN, ISR_DoorSensor_Opened, (void *) DOOR_SENSOR_PIN);
  esp_err_t e2 = gpio_set_intr_type((gpio_num_t)DOOR_SENSOR_PIN, GPIO_INTR_ANYEDGE);
  return e1 > 0 ? e1 : e2;
}

uint8_t door_sensor_read(void) {
  return digitalRead(DOOR_SENSOR_PIN);
}

void door_sensor_task_handle(void) {

  portENTER_CRITICAL_ISR(&doorSensorMux);
  saveTriggered = doorSensorISRTriggered;
  saveDebounceTimeout = doorSensorLastISR;
  saveLastState = doorSensorLastState;
  portEXIT_CRITICAL_ISR(&doorSensorMux);

  bool doorSensorCurrentState = door_sensor_read();
  if (saveTriggered && doorSensorCurrentState == saveLastState && millis() - saveDebounceTimeout > DOOR_SENSOR_DEBOUNCE_MS) {
    if (doorSensorCurrentState == HIGH) { // pull-up / tristate
      push_event_to_queue(EVENT_DOOR_OPENED);
    } else if (doorSensorCurrentState == LOW) { // contact done with GND = closed
      push_event_to_queue(EVENT_DOOR_CLOSED);
    }
    portENTER_CRITICAL_ISR(&doorSensorMux);
    doorSensorISRTriggered = false;
    portEXIT_CRITICAL_ISR(&doorSensorMux);
  }

  // improve behavior in case we missed an ISR
  // read current door state,
  // if LOW and last event is not opened, add new to queue
  // if HIGH and last event is not closed, add new to queue
  // same thing for PIR sensor
  /*if (doorSensorCurrentState == HIGH && last_queued_event != EVENT_DOOR_CLOSED) {
    push_event_to_queue(EVENT_DOOR_CLOSED);
  } else if (doorSensorCurrentState == LOW && last_queued_event != EVENT_DOOR_OPENED) {
    push_event_to_queue(EVENT_DOOR_OPENED);
  }*/
}

uint32_t door_sensor_get_pending_events_count(void) {
  return count_pending_events;
}

dated_event_t door_sensor_get_event(void) {
  dated_event_t * out = pop_event_from_queue();
  if (out) return { .type = out->type, .time = out->time, .id = out->id};
  else return { .type = EVENT_NONE};
}

void IRAM_ATTR ISR_DoorSensor_Opened(void * context) {
  portENTER_CRITICAL_ISR(&doorSensorMux);
  doorSensorLastState = door_sensor_read();
  doorSensorLastISR = millis();
  doorSensorISRTriggered = true;
  portEXIT_CRITICAL_ISR(&doorSensorMux);
}
