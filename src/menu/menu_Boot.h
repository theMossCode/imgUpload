#pragma once

#include "class.h"
#include "menu.h"
#include "../images/images.h"
#include "../display/display.h"
#include "../display/screen_style.h"

#include "../flash/flash.h"


class BootScreen : public Screen
{
  public:
    virtual void pre()
    {
      lv_style_copy(&textStyle, &lv_style_plain);
      textStyle.text.color = ATC_COLOR_DEFAULT;
      textStyle.text.font = &lv_font_roboto_28;

      lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
      lv_label_set_text(label, "ATCwatch");
      lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &textStyle);
      lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
      lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -60); 

      lv_obj_t * img1 = lv_img_create(lv_scr_act(), NULL);
      lv_img_set_src(img1, &boot_img);
      lv_obj_align(img1, NULL, LV_ALIGN_CENTER, 0, 20);
    }

    virtual void main()
    {

    }

    virtual uint32_t sleepTime()
    {
      return 20000;
    }

  private:
  lv_style_t textStyle;
};

BootScreen bootScreen;
