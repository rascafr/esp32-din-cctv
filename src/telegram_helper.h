#ifndef TELEGRAM_HELPER
#define TELEGRAM_HELPER

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

void telegram_helper_send_photo_buffer(WiFiClientSecure * secured_client, uint8_t * fbBuf, size_t fbLen);

#endif