#include "../pinout.h"
#include "../fast_spi.h"
#include "flash.h"

#define RDP_DELAY 1

#define SPIFLASH_PAGE_WRITE 0x02   // Page Program (up to 256 bytes)
#define SPIFLASH_READ 0x03         // Read Data
#define SPI_FLASH_FAST_READ 0x0B   // Fast Read
#define SPIFLASH_READ_STATUS 0x05  // Read Status Register
#define SPIFLASH_WRITE_ENABLE 0x06 // Write Enable
#define SPIFLASH_SECTOR_ERASE 0x20 // SubSector (4K bytes) Erase
#define SPIFLASH_BLOCK_ERASE 0xD8  // Sector (usually 64K bytes) Erase
#define SPIFLASH_RDP 0xAB          // Release from Deep Power Down
#define SPIFLASH_DP 0xB9           // Deep Power Down
#define SPIFLASH_MASS_ERASE 0xC7   // Erase entire flash.
#define SPIFLASH_IDREAD 0x9F
#define SPIFLASH_MACREAD 0x4B

#define WRITE_IN_PROGRESS_FLAG    0x01
#define WRITE_ENABLE_LATCH_FLAG   0x02

enum flash_err{
  FLASH_OK, FLASH_TIMEOUT = -1
};

bool flash_sleeping = false;

//Spiffs
#define LOG_PAGE_SIZE 256

static spiffs flash_fs;
static u8_t spiffs_work_buf[LOG_PAGE_SIZE * 2];
static u8_t spiffs_fds[32 * 10]; 
int is_fs_mounted = SPIFFS_ERR_NOT_MOUNTED;

#if (SPIFFS_CACHE)
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32) + 40];
#define SPIFFS_CACHE_BUFFER       spiffs_cache_buf
#define SPIFFS_CACHE_BUFFER_SIZE  sizeof(spiffs_cache_buf) 
#else
#define SPIFFS_CACHE_BUFFER       NULL
#define SPIFFS_CACHE_BUFFER_SIZE  0
#endif
//End Spiffs

uint8_t temp[8];

static uint8_t read_status_register()
{
  uint8_t temp_status = 0xff;

  temp[0] = SPIFLASH_READ_STATUS;
  startWrite_flash();
  write_fast_spi(temp, 1); 
  do{
    read_fast_spi(temp, 1);
    temp_status = temp[0];
  }while(temp_status == 0xff);

  endWrite_flash();
  return temp_status;
}

static bool set_write_enable_latch()
{
  temp[0] = SPIFLASH_WRITE_ENABLE;
  startWrite_flash();
  write_fast_spi(temp, 1);
  endWrite_flash();

  uint32_t start_millis = millis();
  uint8_t temp_status = read_status_register();
  while (!(temp_status & WRITE_ENABLE_LATCH_FLAG)){
    temp_status = read_status_register();
  }

  return (temp_status & WRITE_ENABLE_LATCH_FLAG);
}

void flash_sleep(int state)
{
  startWrite_flash();

  if (state)
  {
    temp[0] = SPIFLASH_DP;
    write_fast_spi(temp, 1);
  }
  else
  {
    temp[0] = SPIFLASH_RDP;
    write_fast_spi(temp, 1);
    endWrite_flash(false);
    delay(RDP_DELAY);
  }
  endWrite_flash();
  flash_sleeping = state;
}

uint32_t flash_read_id()
{
  startWrite_flash();
  temp[0] = SPIFLASH_IDREAD;
  write_fast_spi(temp, 1);
  read_fast_spi(temp, 3);
  endWrite_flash();

  uint32_t temp_flash_id = 0;
  temp_flash_id |= ((temp[0] << 16) | (temp[1] << 8) | temp[2]);

  return (temp_flash_id);
}

void get_temp(uint8_t *ptr)
{
  ptr[0] = temp[0];
  ptr[1] = temp[1];
  ptr[2] = temp[2];
  ptr[3] = temp[3];
  ptr[4] = temp[4];
  ptr[5] = temp[5];
  ptr[6] = temp[6];
  ptr[7] = temp[7];
}

void startWrite_flash(bool is_enable_spi)
{
  if (is_enable_spi){
    enable_spi(true);
  }
  digitalWrite(SPI_CE, LOW);
}

void endWrite_flash(bool is_disable_spi)
{
  digitalWrite(SPI_CE, HIGH);
  if (is_disable_spi){
    enable_spi(false);
  }
}

void read_flash(int addr, int size, char *buff)
{
  temp[0] = SPI_FLASH_FAST_READ;
  temp[1] = (uint8_t)((addr >> 16) & 0xff);
  temp[2] = (uint8_t)((addr >> 8) & 0xff);
  temp[3] = (uint8_t)(addr & 0xff);
  temp[4] = 0x00; // Dummy byte

  startWrite_flash();
  write_fast_spi(temp, 5);
  read_fast_spi((uint8_t *)buff, (uint32_t)size);
  endWrite_flash();
}

void write_flash(int addr, int size, char *buff)
{
  uint32_t start_millis = millis();
  while (!set_write_enable_latch()){
    ;
  }

  uint8_t temp_buf[size+4];

  temp_buf[0] = SPIFLASH_PAGE_WRITE;
  temp_buf[1] = (uint8_t)((addr >> 16) & 0xff);
  temp_buf[2] = (uint8_t)((addr >> 8) & 0xff);
  temp_buf[3] = (uint8_t)(addr& 0xff);

  for(int i=4; i<(size+4); ++i){
    temp_buf[i] = buff[i-4];
  }

  startWrite_flash();
  write_fast_spi(temp_buf, (size+4));
  endWrite_flash();

  start_millis = millis();
  uint8_t temp_status = read_status_register();
  while (temp_status & WRITE_IN_PROGRESS_FLAG){
    temp_status = read_status_register();
  }
}

void erase_flash(int addr, int size)
{
  uint32_t start_millis = millis();
  while (!set_write_enable_latch()){
    ;
  }

  temp[0] = SPIFLASH_BLOCK_ERASE;
  temp[1] = (uint8_t)((addr >> 16) & 0xff);
  temp[2] = (uint8_t)((addr >> 8) & 0xff);
  temp[3] = (uint8_t)(addr & 0xff);

  startWrite_flash();
  write_fast_spi(temp, 4);
  endWrite_flash();

  start_millis = millis();
  uint8_t temp_status = read_status_register();
  while (temp_status & WRITE_IN_PROGRESS_FLAG){
    temp_status = read_status_register();
  }
}

void init_flash()
{
  pinMode(SPI_CE, OUTPUT);
  digitalWrite(SPI_CE, HIGH);
  flash_sleep(false);

  spiffs_mount();
}

static s32_t spi_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
  read_flash(addr, size, (char *)dst);
  
  return SPIFFS_OK;
}
static s32_t spi_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
  write_flash(addr, size, (char *)src);
  
  return SPIFFS_OK;
}
static s32_t spi_spiffs_erase(u32_t addr, u32_t size)
{
  erase_flash(addr, size);

  return SPIFFS_OK;
}

s32_t spiffs_format()
{
  is_fs_mounted = 0;

  s32_t spiffs_res = spiffs_mount();
  if(spiffs_res != SPIFFS_ERR_NOT_A_FS){
    SPIFFS_unmount(&flash_fs);
  }

  spiffs_res = SPIFFS_format(&flash_fs);
  if(spiffs_res != SPIFFS_OK){
    return SPIFFS_errno(&flash_fs);
  }

  return spiffs_mount();
}

s32_t spiffs_mount()
{
  spiffs_config cfg;
  cfg.hal_read_f = spi_spiffs_read;
  cfg.hal_write_f = spi_spiffs_write;
  cfg.hal_erase_f = spi_spiffs_erase;

  s32_t res = SPIFFS_mount(&flash_fs,
                         &cfg,
                         spiffs_work_buf,
                         spiffs_fds,
                         sizeof(spiffs_fds),
                         SPIFFS_CACHE_BUFFER,
                         SPIFFS_CACHE_BUFFER_SIZE,
                         NULL);

  is_fs_mounted = res;
  return res;
}

bool is_spiffs_mounted()
{
  return (is_fs_mounted == SPIFFS_OK);
}
// End Spiffs

// LVGL FS
static bool is_spiffs_ready(lv_fs_drv_t *drv)
{
  return is_spiffs_mounted();
}

static lv_fs_res_t spiffs_open_file(lv_fs_drv_t *drv, void *file_p, const char *path, lv_fs_mode_t mode)
{
  (void) drv; // Not used
  spiffs_flags open_flags = 0;
  if(mode == (LV_FS_MODE_RD | LV_FS_MODE_WR)){
    open_flags |= SPIFFS_RDWR | SPIFFS_CREAT;
  }
  else if(mode == LV_FS_MODE_RD){
    open_flags |= SPIFFS_RDONLY;
  }
  else if(mode == LV_FS_MODE_WR){
    open_flags |= SPIFFS_WRONLY | SPIFFS_CREAT;
  }

  spiffs_file f = SPIFFS_open(&flash_fs, path, open_flags, 0);
  if(f < 0){
    return LV_FS_RES_UNKNOWN;
  }

  *(spiffs_file *)file_p = f;
  // memcpy(file_p, &f, sizeof(spiffs_file));

  return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_close_file(lv_fs_drv_t *drv, void *file_p)
{
  (void) drv; //Not used
  // spiffs_file *f = (spiffs_file*)file_p;
  s32_t spiffs_res = SPIFFS_close(&flash_fs, *(spiffs_file*)file_p);
  if(spiffs_res < 0){
    return LV_FS_RES_UNKNOWN;
  }

  return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_read_file(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
  (void) drv; //Not used
  // spiffs_file *f = (spiffs_file*)file_p;
  s32_t spiffs_res = SPIFFS_read(&flash_fs, *(spiffs_file*)file_p, (u8_t*)buf, btr);
  if(spiffs_res < 0){
    return LV_FS_RES_UNKNOWN;
  }
  *br = spiffs_res;
  return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_write_file(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
  (void) drv; //Not used
  // spiffs_file *f = (spiffs_file*)file_p;
  s32_t spiffs_res = SPIFFS_write(&flash_fs, *(spiffs_file *)file_p, (u8_t*)buf, btw);
  if(spiffs_res < 0){
    return LV_FS_RES_UNKNOWN;
  }

  *bw = spiffs_res;
  return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos)
{
  (void) drv; //Unused
  // spiffs_file *f = (spiffs_file*)file_p;
  s32_t spiffs_res = SPIFFS_lseek(&flash_fs, *(spiffs_file *)file_p, pos, SPIFFS_SEEK_SET);
  if (spiffs_res < 0){
    return LV_FS_RES_UNKNOWN;
  }

  return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos)
{
  (void) drv; //Unused
  // spiffs_file *f = (spiffs_file*)file_p;
  s32_t spiffs_res = SPIFFS_tell(&flash_fs, *(spiffs_file *)file_p);
  if (spiffs_res < 0){
    return LV_FS_RES_UNKNOWN;
  }

  *pos = spiffs_res;
  return LV_FS_RES_OK;
}

void init_spiffs_lv()
{
  lv_fs_drv_t spiffs_drv; // LVGL FS
  lv_fs_drv_init(&spiffs_drv);

  spiffs_drv.letter = 'S';
  spiffs_drv.file_size = sizeof(spiffs_file);
  spiffs_drv.rddir_size = 0;
  spiffs_drv.ready_cb = is_spiffs_ready;
  spiffs_drv.open_cb = spiffs_open_file;
  spiffs_drv.close_cb = spiffs_close_file;
  spiffs_drv.read_cb = spiffs_read_file;
  spiffs_drv.write_cb = spiffs_write_file;
  spiffs_drv.seek_cb = spiffs_seek;
  spiffs_drv.tell_cb = spiffs_tell;
  spiffs_drv.trunc_cb = NULL;
  spiffs_drv.size_cb = NULL;
  spiffs_drv.rename_cb = NULL;
  spiffs_drv.free_space_cb = NULL;

  spiffs_drv.dir_open_cb = NULL;
  spiffs_drv.dir_read_cb = NULL;
  spiffs_drv.dir_close_cb = NULL;

  lv_fs_drv_register(&spiffs_drv);
}


#if DEBUG
String test_spiffs()
{
    char buf[12];
    // spiffs_file fd = SPIFFS_open(&flash_fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
    // if (SPIFFS_write(&flash_fs, fd, (u8_t *)"Test", 5) < 0) return ("Write Error!" + String(SPIFFS_errno(&flash_fs)));
    // SPIFFS_close(&flash_fs, fd); 
  
    // fd = SPIFFS_open(&flash_fs, "my_file", SPIFFS_RDWR, 0);
    // if (SPIFFS_read(&flash_fs, fd, (u8_t *)buf, 5) < 0) return ("Read Error" + String(SPIFFS_errno(&flash_fs)));
    // SPIFFS_close(&flash_fs, fd);  

    lv_fs_file_t lv_f;
    lv_fs_res_t res;

    const char *file_name = "C:Hello";

    res = lv_fs_open(&lv_f, file_name, LV_FS_MODE_WR | LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK){
      return "Error Opening";
    }

    uint32_t bytes_written = 0;
    res = lv_fs_write(&lv_f, (const char*)"Hello World", 12, &bytes_written);
    if(res |=  LV_FS_RES_OK){
      return "Error Writing";
    }
    lv_fs_close(&lv_f);

    res = lv_fs_open(&lv_f, file_name, LV_FS_MODE_WR | LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK){
      return "Error Opening: R";
    }

    res = lv_fs_read(&lv_f, buf, 12, &bytes_written);
    if(res != LV_FS_RES_OK){
      return "Error reading";
    }

    lv_fs_close(&lv_f);

    String res_str = "";
    for(int i=0; i<12; ++i){
      res_str += String(buf[i]);
    }

    return (res_str + " Read: " + String(bytes_written));
}
#endif

