#include "utils.h"

void utils_write_hostname(char * hostname) {
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(hostname, 23, "ESP32-CAMERA-%04X%08X", chip, (uint32_t)chipid);
}

void utils_init_time(time_t * now) {
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  *now = time(nullptr) + TIMEZONE_OFFSET;
  while (*now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    *now = time(nullptr) + TIMEZONE_OFFSET;
  }
}

time_t utils_get_time(void) {
  return time(nullptr) + TIMEZONE_OFFSET;
}
