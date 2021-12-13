#ifndef UTILS
#define UTILS

#include <Arduino.h>

void utils_write_hostname(char * hostname);
void utils_init_time(time_t * now);
time_t utils_get_time(void);

#endif