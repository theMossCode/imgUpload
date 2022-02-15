#include "menu.h"
#include "../images/images.h"
#include "../pinout.h"
#include "../touch.h"
#include "../display/backlight.h"
#include "../bootloader.h"
#include "../display/display.h"

#include "menu_Boot.h"
#include "menu_Home.h"

long last_main_run;

Screen *currentScreen = &homeScreen;
Screen *oldScreen = &homeScreen;
Screen *lastScreen = &homeScreen;
Screen *baseScreen = &homeScreen;

void init_menu() {

}

void display_home() {
  change_screen(&homeScreen);
}

void display_booting() {
  set_gray_screen_style();
  lastScreen = currentScreen;
  currentScreen = &bootScreen;
  oldScreen = &bootScreen;
  currentScreen->pre_display();
  set_gray_screen_style();
  currentScreen->pre();
  currentScreen->main();
  inc_tick();
  lv_task_handler();
}

void display_screen(bool ignoreWait) {
  if (ignoreWait || millis() - last_main_run > get_menu_delay_time()) {
    last_main_run = millis();
    if (currentScreen != oldScreen) {
      oldScreen->post();
      currentScreen->pre_display();
      set_gray_screen_style();
      oldScreen = currentScreen;
      currentScreen->pre();
    }
    currentScreen->main();
  }
  lv_task_handler();
}

uint32_t get_menu_delay_time() {
  return currentScreen->refreshTime();
}

void change_screen(Screen *screen) {
  if(currentScreen != screen){
    currentScreen = screen;
  }
}

int get_sleep_time_menu() {
  return currentScreen->sleepTime();
}

static void lv_event_handler(lv_obj_t * object, lv_event_t event)
{
  currentScreen->lv_event_class(object, event);
}