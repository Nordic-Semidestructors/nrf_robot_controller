#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "nrf.h"
//#include "nrf_log.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "app_pwm.h"
#include "nrf_gpio.h"

int32_t left_target_speed = 0;
int32_t right_target_speed = 0;

uint32_t left_in0_pin = 0;
uint32_t left_in1_pin = 0;
uint32_t right_in0_pin = 0;
uint32_t right_in1_pin = 0;

// Set to invert outputs. The LEDs on the DK light up when the pin is grounded.
#define DEBUG_WITH_LEDS true

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag = false;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
    //NRF_LOG_INFO("PWM ready.");
}

uint32_t motor_control_init(uint32_t init_left_pwm_pin,
                            uint32_t init_left_in0_pin,
                            uint32_t init_left_in1_pin,
                            uint32_t init_right_pwm_pin,
                            uint32_t init_right_in0_pin,
                            uint32_t init_right_in1_pin)
{
    ret_code_t err_code;


    /* 2-channel PWM, 200Hz, output on DK LED pins. */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, init_left_pwm_pin, init_right_pwm_pin);

    /* Switch the polarity of the second channel. */
    pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);

    left_in0_pin = init_left_in0_pin;
    left_in1_pin = init_left_in1_pin;
    right_in0_pin = init_right_in0_pin;
    right_in1_pin = init_right_in1_pin;

    nrf_gpio_cfg_output(left_in0_pin);
    nrf_gpio_cfg_output(left_in1_pin);
    nrf_gpio_cfg_output(right_in0_pin);
    nrf_gpio_cfg_output(right_in1_pin);

    //NRF_LOG_INFO("Motor control initialized.");

    return NRF_SUCCESS;
}

void motor_control_set(int32_t left_new_speed, int32_t right_new_speed)
{
    bool left_motor_forwards = left_new_speed >= 0;
    bool right_motor_forwards = right_new_speed >= 0;
    
    nrf_gpio_pin_write(left_in0_pin, !left_motor_forwards ^ DEBUG_WITH_LEDS);
    nrf_gpio_pin_write(left_in1_pin, left_motor_forwards ^ DEBUG_WITH_LEDS);
    nrf_gpio_pin_write(right_in0_pin, !right_motor_forwards ^ DEBUG_WITH_LEDS);
    nrf_gpio_pin_write(right_in1_pin, right_motor_forwards ^ DEBUG_WITH_LEDS);

    // Set pwm duty. Ignore return value since this function will be called again soon.
    (void) app_pwm_channel_duty_set(&PWM1, 0, abs(left_new_speed));
    (void) app_pwm_channel_duty_set(&PWM1, 1, abs(right_new_speed));
}
