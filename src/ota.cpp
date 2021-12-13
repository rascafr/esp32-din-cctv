#include "ota.h"
#include <ArduinoOTA.h>

// https://github.com/solcer/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP32/telegramOTA/telegramOTA.ino

volatile bool flag_is_updating = false;

void ota_init_enable(void) {

  ArduinoOTA.onStart([]() {
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    flag_is_updating = true;
  })
  .onEnd([]() {
    flag_is_updating = false;
  })
  .onError([](ota_error_t error) {
    // errors : OTA_AUTH_ERROR, OTA_AUTH_ERROR, OTA_CONNECT_ERROR, OTA_RECEIVE_ERROR, OTA_END_ERROR
    flag_is_updating = false;
  });

  ArduinoOTA.begin();
}

bool ota_is_updating(void) {
  return flag_is_updating;
}

void ota_task_handle(void) {
  ArduinoOTA.handle();
}
