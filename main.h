// main.h
#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/util/queue.h"
#include "hardware/i2c.h"
#include "EEPROM.h"
//#include "GPIOPin.h"
#include "gpiopin.h"
#include "led.h"
using namespace std;

#define I2C i2c0
#define I2C_SDA 16
#define I2C_SDL 17

#define MOTOR_PIN_1 2
#define MOTOR_PIN_2 3
#define MOTOR_PIN_3 6
#define MOTOR_PIN_4 13
#define DELAY 1

#define BUTTON_PIN2 7
#define BUTTON_PIN1 8
#define BUTTON_PIN0 9
#define LED_PIN1 22
#define LED_PIN2 21
#define LED_PIN3 20
#define LIMIT_SWITCH_1 27
#define LIMIT_SWITCH_2 28
#define ROTARY_A 14
#define ROTARY_B 15

#define QUEUE_SIZE 5

#define EEPROM_ADDRESS 0x50
#define MAX_ADDRESS 0xFFFF
#define DOOR_STATE 0x0000
#define ERROR_STATE 0X0003
#define COMPARTMENT_START_STOP_ADDRESS 0X0005
#define CALIBRATION_STATE 0x0007
#define AVERAGE_STEPS_ADDRESS 0x0009
#define ROTARY_DETENTS_ADDRESS 0x000B
#define DOOR_BETWEEN_DIRECTION_ADDRESS 0x000C
#define ADDRESS_SIZE 2
#define FREQUENCY (100*1000)

extern queue_t isr_queue;
extern int new_rotary_count;
extern volatile bool sw1_pressed;
extern uint32_t previous_time;
extern absolute_time_t motor_timeout;

void set_motor_pins(bool reverse);
int motor_calibration(bool reverse, GPIOPin& limit_switch1, GPIOPin& limit_switch2, EEPROM& rotary_obj);
void temp_move(bool reverse);
void open_close(int average_steps, bool reverse,EEPROM& door_state_obj, EEPROM& error_state_obj, uint8_t open_close);
void isr(uint gpio, uint32_t event_mask);
void move_backward(bool reverse, GPIOPin& limit_switch_close, GPIOPin& limit_switch_open, EEPROM& door_state_obj, EEPROM& error_state_obj, EEPROM& door_between_obj);
bool limit_switch_check(GPIOPin& limit_switch);
bool encoder_queue_check();

#endif // MAIN_H
