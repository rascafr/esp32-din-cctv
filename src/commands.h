#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

#define NB_COMMANDS     (10)

// TODO macro-generate /x names directly based on CMD_x from enum below
const char * COMMANDS[NB_COMMANDS] = {
    "/reboot",
    "/capture",
    "/status",
    "/sensors",
    "/beep",
    "/mute",
    "/unmute",
    "/enable",
    "/disable",
    "/debug",
};

typedef enum {
    CMD_REBOOT = 0,
    CMD_CAPTURE,
    CMD_STATUS,
    CMD_SENSORS,
    CMD_BEEP,
    CMD_MUTE,
    CMD_UNMUTE,
    CMD_ENABLE,
    CMD_DISABLE,
    CMD_DEBUG,
} commands_e;

#endif