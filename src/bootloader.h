
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "Arduino.h"

void init_bootloader();
void start_bootloader(bool without_sd=false);
void set_reboot();
bool get_sd_enabled();
void system_off();

#endif