
#include "inputoutput.h"
#include "Arduino.h"
#include "pinout.h"

volatile long vibration_end_time = 0;
volatile long led_end_time = 0;
volatile bool inputoutput_inited = false;
volatile int motor_power = 100;

void init_inputoutput() {
  pinMode(PUSH_BUTTON_IN, INPUT);
  if (PUSH_BUTTON_OUT != -1) {
    pinMode(PUSH_BUTTON_OUT, OUTPUT);
    digitalWrite(PUSH_BUTTON_OUT, HIGH);
  }

  inputoutput_inited = true;
}

bool get_button() {
  if (!inputoutput_inited)init_inputoutput();
  if (PUSH_BUTTON_OUT != -1) {
    pinMode(PUSH_BUTTON_OUT, OUTPUT);
    digitalWrite(PUSH_BUTTON_OUT, HIGH);
  }
  bool button = digitalRead(PUSH_BUTTON_IN);
  return button;
}
