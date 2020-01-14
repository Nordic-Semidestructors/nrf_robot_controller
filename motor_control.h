#ifndef MOTOR_CONTROL_H__
#define MOTOR_CONTROL_H__

uint32_t motor_control_init(uint32_t init_left_pwm_pin,
                            uint32_t init_left_in0_pin,
                            uint32_t init_left_in1_pin,
                            uint32_t init_right_pwm_pin,
                            uint32_t init_right_in0_pin,
                            uint32_t init_right_in1_pin);

void motor_control_set(int32_t left_new_speed, int32_t right_new_speed);

#endif // MOTOR_CONTROL_H__