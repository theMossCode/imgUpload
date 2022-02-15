#include "src/main_app.h"

bool stepsWhereReseted = false;

void setup() {
	delay(500);
	if (get_button()) {// if button is pressed on startup goto Bootloader
		NRF_POWER->GPREGRET = 0x01;
		NVIC_SystemReset();
	}

	init_app();
}

void loop() {
	if (!get_button()){
		watchdog_feed();// reset the watchdog if the push button is not pressed, if it is pressed for more then WATCHDOG timeout the watch will reset
	}
	
	display_screen();// manage menu and display stuff
	
	gets_interrupt_flag();// check interrupt flags and do something with it
}
