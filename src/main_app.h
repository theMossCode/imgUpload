#ifndef MAIN_H
#define MAIN_H

#include "images/images.h"
#include "display/display.h"
#include "display/backlight.h"
#include "display/sleep.h"
#include "menu/menu.h"
#include "inputoutput.h"
#include "watchdog.h"
#include "interrupt.h"
#include "fast_spi.h"
#include "i2c.h"
#include "flash/flash.h"

void init_app();

#endif