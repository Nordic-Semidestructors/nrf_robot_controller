uint32_t motor_control_init(uint32_t left_pwm_pin,
                            uint32_t left_enable_pin,
                            uint32_t left_reverse_pin,
                            uint32_t right_pwm_pin,
                            uint32_t right_enable_pin,
                            uint32_t right_reverse_pin);

void motor_control_set(int32_t left_new_speed, int32_t right_new_speed);