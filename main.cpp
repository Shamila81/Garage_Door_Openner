#include "main.h"

int new_rotary_count = 0;
volatile bool sw1_pressed = false;
uint32_t previous_time = 0;
absolute_time_t motor_timeout = make_timeout_time_ms(400);
queue_t isr_queue;

int main() {
    stdio_init_all();

    queue_init(&isr_queue, sizeof(int),QUEUE_SIZE);

    i2c_init(I2C, FREQUENCY);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SDL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SDL);

    // Initialize GPIO pins
    GPIOPin sw0(BUTTON_PIN0, true, true, true);
    GPIOPin sw1(BUTTON_PIN1, true, true, true);
    GPIOPin sw2(BUTTON_PIN2, true, true, true);
    GPIOPin led1(LED_PIN1, false);
    GPIOPin led2(LED_PIN2, false);
    GPIOPin led3(LED_PIN3, false);
    GPIOPin motor_pin1(MOTOR_PIN_1, false);
    GPIOPin motor_pin2(MOTOR_PIN_2, false);
    GPIOPin motor_pin3(MOTOR_PIN_3, false);
    GPIOPin motor_pin4(MOTOR_PIN_4, false);
    GPIOPin limit_switch_close(LIMIT_SWITCH_1, true, true, true);
    GPIOPin limit_switch_open(LIMIT_SWITCH_2, true, true, true);
    GPIOPin rotary_A(ROTARY_A, true, true, true);
    GPIOPin rotary_B(ROTARY_B, true, true, true);

    Button button_sw0(&sw0);
    Button button_sw1(&sw1);
    Button button_sw2(&sw2);

    ButtonManager button_manager(&button_sw0, &button_sw2);

    LED led_controller(&led1, &led2, &led3);

    // Initialize EEPROM objects
    uint8_t door_state_data = 0xFF;
    uint8_t error_state_data = 0xFF;
    uint8_t calibration_state_data = 0xFF;
    uint16_t average_steps_data = 0xFF;
    uint8_t rotary_detents_data = 0xFF;
    uint8_t door_between_direction_data;

    uint8_t door_open = 0x00;
    uint8_t door_close = 0x01;
    uint8_t door_between = 0x02;
    uint8_t error_yes = 0x00;
    uint8_t error_no = 0x01;
    uint8_t calib_yes = 0x00;
    uint8_t calib_no = 0x01;
    uint8_t reverse_true = 0x00;
    uint8_t reverse_false = 0x01;

    EEPROM door_state_obj(DOOR_STATE, &door_state_data, sizeof(door_state_data));
    EEPROM error_state_obj(ERROR_STATE, &error_state_data, sizeof(error_state_data));
    EEPROM calibration_state_obj(CALIBRATION_STATE, &calibration_state_data, sizeof(calibration_state_data));
    EEPROM average_state_obj(AVERAGE_STEPS_ADDRESS, &average_steps_data, sizeof(average_steps_data));
    EEPROM rotary_state_obj(ROTARY_DETENTS_ADDRESS, &rotary_detents_data, sizeof(rotary_detents_data));
    EEPROM door_between_direction_obj(DOOR_BETWEEN_DIRECTION_ADDRESS,&door_between_direction_data, sizeof(door_between_direction_data));

    gpio_set_irq_enabled_with_callback(ROTARY_A, GPIO_IRQ_EDGE_RISE, true, isr);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN1, GPIO_IRQ_EDGE_FALL, true, isr);

    // motor to rotary --> close(reverse true)
    int state = 1;
    bool reverse = true;
    int average_steps=0;

    //door_state_obj.eeprom_write_state(&door_open);
    //error_state_obj.eeprom_write_state(&initial);
    //calibration_state_obj.eeprom_write_state(&calib_yes);
    //error_state_obj.eeprom_write_state(&error_no);
    door_state_obj.eeprom_read_state();
    error_state_obj.eeprom_read_state();
    calibration_state_obj.eeprom_read_state();
    average_state_obj.eeprom_read_state();
    rotary_state_obj.eeprom_read_state();
    door_between_direction_obj.eeprom_read_state();

    if(calibration_state_data == calib_no || error_state_data == error_yes) {
        cout<<"Door should be calibrated!"<<endl;
        state = 1;
    }else if(calibration_state_data == calib_yes) {
        state = 2;
    }else if(door_state_data == door_between) {
        state = 2;
    }

    led1.write(false);
    led2.write(false);
    led3.write(false);
    if(door_state_data == door_between) {
        cout<<"Door in between"<<endl;
        led2.write(true);
    }else if(door_state_data == door_open) {
        cout<<"Door opened!"<<endl;
        led1.write(true);
    }else if(door_state_data == door_close) {
        cout<<"Door closed!"<<endl;
        led3.write(true);
    }

    while(true) {
        switch (state) {
            case 1: {

                if(error_state_data == error_yes) {
                    //blinking(led1,led2,led3, sw0, sw2);
                    blinking(led_controller, button_sw0, button_sw2);
                }

                if(button_manager.both_pressed()) {
                    sleep_ms(50);
                    led_controller.turn_all_off();
                    motor_timeout = make_timeout_time_ms(400);
                    reverse = true;
                    cout<<"Calibrating..."<<endl;
                    calibration_state_obj.eeprom_read_write(&calib_no);
                    average_steps = motor_calibration(reverse,limit_switch_close, limit_switch_open, calibration_state_obj);

                    if(calibration_state_data == calib_yes) {
                        average_state_obj.eeprom_read_write(&average_steps);
                        error_state_obj.eeprom_read_write(&error_no);
                        door_state_obj.eeprom_read_write(&door_close);
                        cout<<"Calibration completed!"<<endl;
                        led_controller.turn_all_on();
                        state = 2;
                    }else {
                        state = 1;
                    }
                }
                break;
            }
            case 2: {
                while(true) {
                    if(button_sw1.is_pressed()) {
                        sleep_ms(50);

                        if(door_state_data==door_close) {

                            reverse = false;
                            door_state_obj.eeprom_read_write(&door_between);
                            error_state_obj.eeprom_read_write(&error_yes);
                            cout<<"Door opening ..."<<endl;
                            led_controller.turn_all_off();
                            open_close(average_steps_data,reverse, door_state_obj,error_state_obj,door_open);

                            if(error_state_data == error_yes) {
                                state = 1;
                                break;
                            }else {
                                if(door_state_data == door_between) {

                                    led_controller.set_custom(false,true,false);
                                }else {
                                    cout<<"Door open!"<<endl;
                                    led_controller.set_custom(true,false,false);
                                }
                                door_between_direction_obj.eeprom_read_write(&reverse_false);
                            }

                        }else if(door_state_data==door_open) {
                            reverse = true;
                            door_state_obj.eeprom_read_write(&door_between);
                            error_state_obj.eeprom_read_write(&error_yes);
                            cout<<"Door closing ...."<<endl;
                            led_controller.turn_all_off();
                            open_close(average_steps_data,reverse, door_state_obj, error_state_obj,door_close);

                            if(error_state_data == error_yes) {
                                state = 1;
                                break;
                            }else {
                                if(door_state_data == door_between) {
                                    led_controller.set_custom(false, true, false);
                                }else {
                                    cout<<"Door close!"<<endl;
                                    led_controller.set_custom(false, false, true);
                                }
                                door_between_direction_obj.eeprom_read_write(&reverse_true);
                            }

                        }else if(door_state_data==door_between) {
                            cout<<"Door moving..."<<endl;
                            led_controller.set_custom(false, true, false);
                            error_state_obj.eeprom_read_write(&error_yes);

                            if(door_between_direction_data==reverse_true) {
                                reverse = false;
                            }else {
                                reverse = true;
                            }
                            move_backward(reverse, limit_switch_close,limit_switch_open, door_state_obj, error_state_obj, door_between_direction_obj);
                            if(error_state_data == error_yes) {
                                state = 1;
                                break;
                            }
                            if(door_state_data == door_between) {
                                led_controller.set_custom(false, true,false);
                            }else if(door_state_data == door_close) {
                                led_controller.set_custom(false, false,true);
                            }else if(door_state_data == door_open) {
                                led_controller.set_custom(true, false,false);
                            }
                        }
                    }
                    sleep_ms(20);
                }
                break;
            }
        }
    }
}