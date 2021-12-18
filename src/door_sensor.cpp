#include "door_sensor.h"
#include "utils.h"

/*
 * Mutexes
 */
portMUX_TYPE doorSensorMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE doorbellMux = portMUX_INITIALIZER_UNLOCKED;

/*
 * ISR variables 
 */
volatile uint32_t doorSensorLastISR = 0;
volatile bool doorSensorLastState = true;
volatile bool doorSensorISRTriggered = false;
volatile uint32_t doorbellLastISR = 0;
volatile uint8_t doorbellSignalEdges = 0;
volatile bool doorbellISRTriggered = false;

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
uint32_t count_pending_events = 0;  // cannot be > MAX_EVENTS_COUNT
static uint8_t __id = 0;            // for debugging purpose, event ID

void push_event_to_queue(sensors_events_e new_event) {
  if (count_pending_events > 0) {
    for (uint8_t i=count_pending_events;i>0;i--) {
      memcpy(&events_list[i], &events_list[i-1], sizeof(dated_event_t));
    }
  }
  events_list[0] = { .type = new_event, .time = utils_get_time(), .id = __id++ };
  if (count_pending_events < MAX_EVENTS_COUNT) count_pending_events++;
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
void IRAM_ATTR ISR_door_sensor_events(void * context);
void IRAM_ATTR ISR_door_bell_events(void * context);

esp_err_t door_init(void) {
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(DOORBELL_PIN, INPUT_PULLUP);
  esp_err_t e1 = gpio_isr_handler_add((gpio_num_t)DOOR_SENSOR_PIN, ISR_door_sensor_events, (void *) DOOR_SENSOR_PIN);
  esp_err_t e2 = gpio_set_intr_type((gpio_num_t)DOOR_SENSOR_PIN, GPIO_INTR_ANYEDGE);
  esp_err_t e3 = gpio_isr_handler_add((gpio_num_t)DOORBELL_PIN, ISR_door_bell_events, (void *) DOORBELL_PIN);
  esp_err_t e4 = gpio_set_intr_type((gpio_num_t)DOORBELL_PIN, GPIO_INTR_NEGEDGE);
  return !e1 && !e2 && !e3 && !e4;
}

uint8_t door_sensor_read(void) {
  return digitalRead(DOOR_SENSOR_PIN);
}

uint8_t door_bell_read(void) {
  return digitalRead(DOORBELL_PIN);
}

void door_task_handle(void) {

  /*
   * Door reed switch true interrupt / debounce detection
   * both states are detected (opened, closed)
   */
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

  /*
   * Doorbell incoming AC / 50Hz square signal
   * only pressed state (signal detected) is managed here
   * (who needs to know when the doorbell button has been released?)
   */
  portENTER_CRITICAL_ISR(&doorbellMux);
  saveTriggered = doorbellISRTriggered;
  portEXIT_CRITICAL_ISR(&doorbellMux);
  if (saveTriggered) {
    push_event_to_queue(EVENT_DOORBELL_PRESSED);
  }
  portENTER_CRITICAL_ISR(&doorbellMux);
  doorbellISRTriggered = false;
  doorbellSignalEdges = 0; // reset 50Hz pulse counter
  portEXIT_CRITICAL_ISR(&doorbellMux);
}

uint32_t door_count_pending_events(void) {
  return count_pending_events;
}

uint8_t door_count_total_events(void) {
  return __id;
}

dated_event_t door_get_event(void) {
  dated_event_t * out = pop_event_from_queue();
  if (out) return { .type = out->type, .time = out->time, .id = out->id };
  else return { .type = EVENT_NONE};
}

void IRAM_ATTR ISR_door_sensor_events(void * context) {
  portENTER_CRITICAL_ISR(&doorSensorMux);
  doorSensorLastState = door_sensor_read();
  doorSensorLastISR = millis();
  doorSensorISRTriggered = true;
  portEXIT_CRITICAL_ISR(&doorSensorMux);
}

void IRAM_ATTR ISR_door_bell_events(void * context) {
  portENTER_CRITICAL_ISR(&doorbellMux);
  doorbellSignalEdges++; // count falling edges
  // check flag here, since the doorbell signal is 50Hz squared, don't disable interrupt
  if (!doorbellISRTriggered && doorbellSignalEdges > DOORBELL_MIN_50HZ_PULSES && millis() - doorbellLastISR > DOORBELL_DEBOUNCE_MS) {
    doorbellISRTriggered = true;
    doorbellLastISR = millis();
  }
  portEXIT_CRITICAL_ISR(&doorbellMux);
}
