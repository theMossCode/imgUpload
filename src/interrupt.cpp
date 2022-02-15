
#include "interrupt.h"
#include "pinout.h"
#include "i2c.h"
#include "menu/menu.h"
#include "display/sleep.h"
#include "touch.h"
#include "inputoutput.h"

long last_button_press = 0;

bool interrupt_enabled = false;

volatile bool button_int;
volatile bool touch_int;

volatile bool last_touch_state;
volatile bool last_button_state;

#ifdef __cplusplus
extern "C" {
#endif
void GPIOTE_IRQHandler()
{
  if ((NRF_GPIOTE->EVENTS_PORT != 0))
  {
    NRF_GPIOTE->EVENTS_PORT = 0;

    bool button_pin = digitalRead(PUSH_BUTTON_IN);
    if (button_pin != last_button_state) {
      last_button_state = button_pin;
      NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] &= ~GPIO_PIN_CNF_SENSE_Msk;
      NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] |= ((last_button_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
      set_button_interrupt();
    }
    bool touch_pin = digitalRead(TP_INT);
    if (touch_pin != last_touch_state) {
      last_touch_state = touch_pin;
      NRF_GPIO->PIN_CNF[TP_INT] &= ~GPIO_PIN_CNF_SENSE_Msk;
      NRF_GPIO->PIN_CNF[TP_INT] |= ((last_touch_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
      if (last_touch_state == false)set_touch_interrupt();
    }
  }
  (void)NRF_GPIOTE->EVENTS_PORT;
}
#ifdef __cplusplus
}
#endif

void init_interrupt() {

  NRF_GPIOTE->INTENCLR = GPIOTE_INTENSET_PORT_Msk;
  NVIC_DisableIRQ(GPIOTE_IRQn);
  NVIC_ClearPendingIRQ(GPIOTE_IRQn);
  NVIC_SetPriority(GPIOTE_IRQn, 1);
  NVIC_EnableIRQ(GPIOTE_IRQn);


  NRF_GPIOTE->EVENTS_PORT = 1;
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;

  if (PUSH_BUTTON_OUT != -1) {
    pinMode(PUSH_BUTTON_OUT, OUTPUT);
    digitalWrite(PUSH_BUTTON_OUT, HIGH);
  }
  pinMode(PUSH_BUTTON_IN, INPUT );
  last_button_state = digitalRead(PUSH_BUTTON_IN);
  NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] |= ((uint32_t)  (last_button_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High)  << GPIO_PIN_CNF_SENSE_Pos);

  pinMode(TP_INT, INPUT);
  last_touch_state = digitalRead(TP_INT);
  NRF_GPIO->PIN_CNF[TP_INT] |= ((uint32_t)  (last_touch_state ? GPIO_PIN_CNF_SENSE_Low : GPIO_PIN_CNF_SENSE_High) << GPIO_PIN_CNF_SENSE_Pos);
  interrupt_enabled = true;
}

void gets_interrupt_flag() {
  if (button_int) {
    button_int = false;
    interrupt_button();
  }
  if (touch_int) {
    touch_int = false;
    interrupt_touch();
  }
}

void set_button_interrupt() {
  button_int = true;
}

void set_touch_interrupt() {
  if (!get_i2cReading() && !get_sleep())get_read_touch();
  touch_int = true;
}

void interrupt_button() {
  if (get_button() && (millis() - last_button_press > 200)) {
    last_button_press = millis();
    if (!sleep_up(WAKEUP_BUTTON)) {
      display_home();
    }
    set_sleep_time();
  }
}

void interrupt_touch() {
  set_was_touched(true);
  if (!sleep_up(WAKEUP_TOUCH)) {
    touch_data_struct touch_data = get_touch();
    set_new_touch_interrupt();
  }
  set_sleep_time();
}

void disable_interrupt() {
  NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] &= ~GPIO_PIN_CNF_SENSE_Msk;
  NRF_GPIO->PIN_CNF[PUSH_BUTTON_IN] |= (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

  NRF_GPIO->PIN_CNF[TP_INT] &= ~GPIO_PIN_CNF_SENSE_Msk;
  NRF_GPIO->PIN_CNF[TP_INT] |= (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

  interrupt_enabled = false;
}
