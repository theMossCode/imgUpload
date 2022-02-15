#pragma once

#include "Arduino.h"
#include <lvgl.h>

#define ATC_COLOR_MILK	  lv_color_hsv_to_rgb(10, 5, 95)	
#define ATC_COLOR_DEFAULT lv_color_hex(0xe6e6e6)

lv_color_t color_list[] = {ATC_COLOR_DEFAULT, ATC_COLOR_MILK, LV_COLOR_GRAY, LV_COLOR_BLACK, LV_COLOR_RED, LV_COLOR_GREEN, LV_COLOR_BLUE};

int main_color_save = 3;
int grad_color_save = 3;
int font_color_save = 0;

lv_style_t global_screen_style;

void set_gray_screen_style( lv_font_t * font = &lv_font_roboto_22) {
  lv_style_copy(&global_screen_style, &lv_style_plain);
  global_screen_style.body.main_color = color_list[main_color_save];// LV_COLOR_GREEN;
  global_screen_style.body.grad_color = color_list[grad_color_save];//LV_COLOR_BLUE;
  global_screen_style.text.color = color_list[font_color_save];
  global_screen_style.text.font = font;
  lv_obj_set_style(lv_scr_act(), &global_screen_style);
}


void set_main_color(int new_color) {
  main_color_save = new_color;
}

void set_grad_color(int new_color) {
  grad_color_save = new_color;
}

void set_font_color(int new_color) {
  font_color_save = new_color;
}

int get_main_color() {
  return main_color_save;
}

int get_grad_color() {
  return grad_color_save;
}

int get_font_color() {
  return font_color_save;
}
