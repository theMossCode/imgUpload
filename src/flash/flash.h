
#ifndef FLASH_H
#define FLASH_H

#include <Arduino.h>
#include <lvgl.h>
#include "spiffs/spiffs.h"


void init_flash();
void flash_sleep(int state);
uint32_t flash_read_id();
void get_temp(uint8_t *ptr);
void startWrite_flash(bool is_enable_spi=true);
void endWrite_flash(bool is_disable_spi=true);

static bool set_write_enable_latch();
static uint8_t read_status_register();

void read_flash(int addr, int size, char *buf);
void write_flash(int addr, int size, char *buf);
void erase_flash(int addr, int size);

// Spiffs callbacks
static s32_t spi_spiffs_read(u32_t addr, u32_t size, u8_t *dst);
static s32_t spi_spiffs_write(u32_t addr, u32_t size, u8_t *src);
static s32_t spi_spiffs_erase(u32_t addr, u32_t size);

s32_t spiffs_format();
s32_t spiffs_mount();
bool is_spiffs_mounted();

// LVGL File system callbacks
void init_spiffs_lv();
static lv_fs_res_t spiffs_open_file(lv_fs_drv_t *drv, void *file_p, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t spiffs_close_file(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t spiffs_read_file(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t spiffs_write_file(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t spiffs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos);
static lv_fs_res_t spiffs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos);
#if DEBUG
String test_spiffs();
#endif //DEBUG

#endif