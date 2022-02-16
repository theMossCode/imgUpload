#pragma once

#include "class.h"
#include "menu.h"
#include "../images/images.h"
#include "../display/display.h"
#include "lvgl.h"

enum
{
  state_init,
  state_format,
  state_mounted,
  state_writing,
  state_reading,
  state_complete
};

enum{
  reading_init,
  reading_preview,
  reading_complete
};

typedef struct{
  const char *src;
  const char *name;
  uint16_t size;
}img_bin_t;

const char *about_src;
const char *heart_src;
const char *message_src;
const char *settings_src;
const char *steps_src;

class HomeScreen : public Screen
{
public:
  virtual void pre()
  {
    // Create img array
    const char *src = NULL;

    about.name = "about.bin";
    about.size = get_img_about_bin_size();
    about.src = (const char*)get_img_about_bin_src();

    heart.name = "heart.bin";
    heart.size = get_img_heart_bin_size();
    heart.src = (const char*)get_img_heart_bin_src();;

    message.name = "message.bin";
    message.size = get_img_message_bin_size();
    message.src = (const char*)get_img_message_bin_src();;

    settings.name = "settings.bin";
    settings.size = get_img_settings_bin_size();
    settings.src = (const char*)get_img_settings_bin_src();;

    steps.name = "steps.bin";
    steps.size = get_img_steps_bin_size();
    steps.src = (const char*)get_img_steps_bin_src();;

    images[0] = about;
    images[1] = heart;
    images[2] = message;
    images[3] = settings;
    images[4] = steps;

    set_gray_screen_style();

    state = state_init;
    curr_img = 0;

    main_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_align(main_label, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 10);
    lv_label_set_align(main_label, LV_LABEL_ALIGN_LEFT);
    lv_label_set_text(main_label, "Init...");

    img_preview = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(img_preview, &boot_img);
    lv_obj_align(img_preview, NULL, LV_ALIGN_CENTER, 0, 20);

    spiffs_format();
  }

  virtual void main()
  {
    switch (state)
    {
    case state_init:
    {
      lv_label_set_text(main_label, "Formatting Flash...");
      state = state_format;
      break;
    }
    case state_format:
    {
      // spiffs_format();
      if (!is_spiffs_mounted())
      {
        lv_label_set_text(main_label, LV_SYMBOL_CLOSE " Flash Init Error!\r\nPress and hold button to reboot");
        state = state_complete;
      }
      else
      {
        lv_label_set_text(main_label, LV_SYMBOL_OK "Initialising LV FS");
        state = state_mounted;
      }
      break;
    }
    case state_mounted:
    {
      init_spiffs_lv();
      lv_label_set_text_fmt(main_label, "writing image:\n%s\nSize: %d", images[curr_img].name, 
                            images[curr_img].size);
      // lv_img_set_src(img_preview, images[curr_img].img_dsc);

      state = state_writing;
      break;
    }
    case state_writing:
    {
      lv_fs_res_t write_res = LV_FS_RES_OK;
      write_res = this->write_img(images[curr_img].name, images[curr_img].src, images[curr_img].size);
      if(write_res == LV_FS_RES_OK){
        if(++curr_img >= (sizeof(images) / sizeof(img_bin_t))){
          curr_img = 0;
          reading_state = reading_init;
          state = state_reading;
        }
        else{
          state = state_mounted;
        }
      }
      else{
        lv_label_set_text_fmt(main_label, LV_SYMBOL_CLOSE" Write Error!\r\nCode %d", write_res);
        state = state_complete;
      }
      break;
    }
    case state_reading:{
      switch (reading_state)
      {
      case reading_init:{
        // display stuff
        lv_label_set_text_fmt(main_label, "Reading from Flash\n%s", (String(img_dir) + String(images[curr_img].name)).c_str());
        reading_state = reading_preview;
        break;
      }
      case reading_preview:{
        lv_img_set_src(img_preview, (String(img_dir) + String(images[curr_img].name)).c_str());

        if(++curr_img >= (sizeof(images)/sizeof(img_bin_t))){
          curr_img = 0;
          reading_state = reading_complete;
        }
        else{
          reading_state = reading_init;
        }
        break;
      }
      default:{
        delay(1000);
        lv_label_set_text(main_label, "Program Flash Success :)");
        state = state_complete;
        break;
      }
      }
      break;
    }
    default:
    {
      // Do notthing
    }
    }
  }

  virtual void up()
  {
  }

  virtual void down()
  {
  }

  virtual void left()
  {
  }

  virtual void right()
  {
  }

private:
img_bin_t about;
img_bin_t heart;
img_bin_t message;
img_bin_t settings;
img_bin_t steps;

img_bin_t images[5];

  int state, reading_state, curr_img;
  lv_obj_t *main_label, *img_preview;

  const char *img_dir = "S:img/";

  lv_fs_res_t write_img(const char *name, const char *src, uint16_t size)
  {
    lv_fs_file_t img_file;
    lv_fs_res_t res = lv_fs_open(&img_file, (String(img_dir) + String(name)).c_str(), LV_FS_MODE_WR);
    if (res != LV_FS_RES_OK)
    {
      return res;
    }

    uint32_t bw = 0;
    res = lv_fs_write(&img_file, src, size, &bw);
    if (res != LV_FS_RES_OK)
    {
      lv_fs_close(&img_file);
      return res;
    }

    lv_fs_close(&img_file);

    if (bw == size)
    {
      return LV_FS_RES_OK;
    }

    return LV_FS_RES_UNKNOWN;
  }
};

HomeScreen homeScreen;
