// motor_functions.cpp
#include "main.h"
#include "GPIOPin.h"

void set_motor_pins(bool reverse) {
    const int half_step_sequence[8][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };

    static int step_index = 0;

    if (reverse) {
        step_index = (step_index - 1 + 8) % 8;
    } else {
        step_index = (step_index + 1) % 8;
    }

    gpio_put(MOTOR_PIN_1, half_step_sequence[step_index][0]);
    gpio_put(MOTOR_PIN_2, half_step_sequence[step_index][1]);
    gpio_put(MOTOR_PIN_3, half_step_sequence[step_index][2]);
    gpio_put(MOTOR_PIN_4, half_step_sequence[step_index][3]);

    sleep_ms(DELAY);
}

bool limit_switch_check(GPIOPin& limit_switch) {
    bool limit_switch_hit = false;
    if (limit_switch.read() && !limit_switch_hit) {
        limit_switch_hit = true;
        sleep_ms(500);

    }else if(!limit_switch.read()) {
        limit_switch_hit= false;
    }

    return limit_switch_hit;
}

int motor_calibration(bool reverse, GPIOPin& limit_switch_close, GPIOPin& limit_switch_open, EEPROM& calibration_state_obj) {
    int count = 0;
    int steps_count = 0;
    bool stuck = false;
    int average_count;
    bool limit_switch_hit_close = false;
    bool limit_switch_hit_open = false;
    uint8_t calib_yes = 0x00;

    while (!stuck) {
        stuck = encoder_queue_check();
        if(count ==3) {
            calibration_state_obj.eeprom_read_write(&calib_yes);
            break;
        }
        if (limit_switch_close.read() && !limit_switch_hit_close) {
            count++;
            new_rotary_count = 0;
            previous_time = 0;
            reverse = !reverse;
            limit_switch_hit_close = true;
            sleep_ms(500);
            motor_timeout = make_timeout_time_ms(400);
        }else if(!limit_switch_close.read()) {
            limit_switch_hit_close = false;
        }
        if (limit_switch_open.read() && !limit_switch_hit_open) {
            count++;
            reverse = !reverse;
            limit_switch_hit_open = true;
            sleep_ms(500);
            motor_timeout = make_timeout_time_ms(400);

        }else if(!limit_switch_open.read()) {
            limit_switch_hit_open = false;
        }

        if(count>=1) {
            steps_count++;
        }
        set_motor_pins(reverse);
    }

    average_count = steps_count/(count-1);
    return average_count;
}

void temp_move(bool reverse) {
    while (true) {
        set_motor_pins(reverse);
    }
}

void open_close(int average_steps, bool reverse, EEPROM& door_state_obj, EEPROM& error_state_obj, uint8_t open_close) {
    int count = 0;
    int press_count=0;
    bool stuck = false;
    bool stop = false;
    uint8_t error_no = 0x01;
    previous_time = 0;
    sleep_ms(100);
    motor_timeout = make_timeout_time_ms(400);
    sw1_pressed = false;
    while (!stop && !stuck) {
        stuck = encoder_queue_check();
        if(sw1_pressed) {
            sleep_ms(50);
            press_count++;
            cout << "Motor stopped!"<< endl;
            error_state_obj.eeprom_read_write(&error_no);
            stop = true;
            sw1_pressed = false;

        }else if(count >= average_steps){
            door_state_obj.eeprom_read_write(&open_close);
            error_state_obj.eeprom_read_write(&error_no);
            stop =true;

        } else {
            set_motor_pins(reverse);
            count++;
        }
    }
}

void isr(uint gpio, uint32_t event_mask) {
    if(gpio == 8) {
        static uint32_t last_press_time = 0;
        uint32_t current_time = time_us_64();
        if ((current_time - last_press_time) > 400000) {
            sw1_pressed = true;
            last_press_time = current_time;
        }
    }else {
        int no =1;
        queue_try_add(&isr_queue,&no);
    }
}

bool encoder_queue_check() {
    int current_value;

    if(queue_try_remove(&isr_queue, &current_value)) {
        motor_timeout = make_timeout_time_ms(400);
    }

    if(time_reached(motor_timeout)){
        cout<<"Door stuck. Calibrate again!"<<endl;
        return true;
    }

    return false;
}

void move_backward(bool reverse, GPIOPin& limit_switch_close, GPIOPin& limit_switch_open, EEPROM& door_state_obj, EEPROM& error_state_obj, EEPROM& door_between_obj) {
    uint8_t door_open = 0x00;
    uint8_t door_close = 0x01;
    uint8_t error_no = 0x01;
    uint8_t reverse_true = 0x00;
    uint8_t reverse_false = 0x01;
    bool stuck = false;
    previous_time = 0;
    sleep_ms(100);
    sw1_pressed = false;
    motor_timeout = make_timeout_time_ms(400);
    if(reverse) {
        while(!stuck) {
            stuck = encoder_queue_check();
            if(sw1_pressed) {
                sleep_ms(50);
                door_between_obj.eeprom_read_write(&reverse_true);
                sw1_pressed = false;
                cout << "Motor stopped!"<< endl;
                error_state_obj.eeprom_read_write(&error_no);
                break;
            }
            if(limit_switch_check(limit_switch_close)) {
                cout<<"Door closed!"<<endl;
                door_state_obj.eeprom_read_write(&door_close);
                error_state_obj.eeprom_read_write(&error_no);
                break;
            }
            set_motor_pins(reverse);
        }
    }else {
        while(!stuck) {
            stuck = encoder_queue_check();
            if(sw1_pressed) {
                sleep_ms(50);
                sw1_pressed = false;
                door_between_obj.eeprom_read_write(&reverse_false);
                error_state_obj.eeprom_read_write(&error_no);
                cout << "Motor stopped!"<< endl;
                break;
            }
            if(limit_switch_check(limit_switch_open)) {
                cout<<"Door Opened!"<<endl;
                door_state_obj.eeprom_read_write(&door_open);
                error_state_obj.eeprom_read_write(&error_no);
                break;
            }
            set_motor_pins(reverse);
        }
    }
}
