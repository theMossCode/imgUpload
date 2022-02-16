#pragma once

#include "class.h"
#include "menu.h"
#include "../images/images.h"
#include "../display/display.h"
#include "../display/screen_style.h"
#include "../flash/flash.h"

enum{
  init_flash_init,
  init_flash_mount,
  init_flash_format,
  init_flash_error,
};


class BootScreen : public Screen
{
  public:
    virtual void pre()
    {
      lv_style_copy(&textStyle, &lv_style_plain);
      textStyle.text.color = ATC_COLOR_DEFAULT;
      textStyle.text.font = &lv_font_roboto_28;

      init_flash_state = init_flash_init;
      retry_counter = start_format = 0;

      label = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_text(label, "ATCwatch");
      lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &textStyle);
      lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -60); 

      img1 = lv_img_create(lv_scr_act(), NULL);
      lv_img_set_src(img1, &boot_img);
      lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 20);
    }

    virtual void main()
    {
      switch (init_flash_state)
      {
      case init_flash_init:{
        if(!is_spiffs_mounted()){
          textStyle.text.font = &lv_font_roboto_22;
          lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &textStyle);
          lv_obj_align(label, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
          lv_label_set_text_fmt(label, "Flash mount error!\nRetrying.. %d", retry_counter+1);
          init_flash_state = init_flash_mount;
        }
        else{
          display_home();
        }
        break;
      }    
      case init_flash_mount:{
        delay(500);
        if(spiffs_mount() != SPIFFS_OK){
          retry_counter++;
        }

        if(retry_counter >= 10){
          init_flash_state = init_flash_format;
          break;
        }

        init_flash_state = init_flash_init;
        break;
      }
      case init_flash_format:{
        lv_label_set_text(label, "Error mounting flash\nAttempting format...");

        if(!start_format){
          start_format = 1;
        }
        else{
          if(spiffs_format() != SPIFFS_OK){
            init_flash_state = init_flash_error;
          }
          else{
            init_flash_state = init_flash_init;
          }
        }
        break;
      }
      default:{
        lv_label_set_text(label, "Error Initialising flash!");
        break;
      }
      }
    }

    virtual uint32_t sleepTime()
    {
      return 300000;
    }

  private:
  lv_style_t textStyle;
  lv_obj_t *label, *img1;

  uint8_t init_flash_state, retry_counter, start_format;
};

BootScreen bootScreen;
