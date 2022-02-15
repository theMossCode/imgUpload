
#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include "Arduino.h"

void init_backlight();
void set_backlight();
int get_backlight();
void set_backlight(int brightness);
void inc_backlight();
void dec_backlight();


#endif