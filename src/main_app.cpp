#include "main_app.h"

void init_app()
{
	init_watchdog();// Init all kind of hardware and software
	initRTC2();
	init_fast_spi();// needs to be before init_display and external flash
	init_i2c();// needs to be before init_hrs3300, init_touch and init_accl
	init_backlight();
	init_display();
	display_booting();
	set_backlight(3);
	init_flash();
	init_touch();
	init_sleep();
	init_menu();
	init_interrupt();// must be after ble!!!
	set_backlight(1);
	display_home();    
}