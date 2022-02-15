#pragma once

#include "Arduino.h"
#include "../touch.h"
#include "class.h"
#include <lvgl.h>

void init_menu();
void display_home();
void display_notify();
void display_charging();
void display_booting();
void display_mb_reboot();
void display_mb_bootloader();
void display_mb_shutdown();
void display_screen(bool ignoreWait = false);
void display_last_screen();

void display_bledetected();

void check_menu(touch_data_struct touch_data);
uint32_t get_menu_delay_time();
int get_sleep_time_menu();
void change_screen(Screen *screen);
void set_last_menu();
void set_last_screen(Screen *screen = NULL);
void set_swipe_enabled(bool state);
bool swipe_enabled();
void inc_vars_menu();
void dec_vars_menu();
static void lv_event_handler(lv_obj_t * object, lv_event_t event);

void display_wakeup_alarm_notification();
void display_battery_notify();
void display_antiloss_notify();
void display_ble_disconnect_push();