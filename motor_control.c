#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "app_pwm.h"

int32_t left_target_speed = 0;
int32_t right_target_speed = 0;

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, 50));
    APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, 99));
    NRF_LOG("PWM ready.");
}

uint32_t motor_control_init(uint32_t left_pwm_pin,
                            uint32_t left_enable_pin,
                            uint32_t left_reverse_pin,
                            uint32_t right_pwm_pin,
                            uint32_t right_enable_pin,
                            uint32_t right_reverse_pin)
{
    ret_code_t err_code;

    /* 2-channel PWM, 200Hz, output on DK LED pins. */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, BSP_LED_0, BSP_LED_1);

    /* Switch the polarity of the second channel. */
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);

    NRF_LOG("Motor control initialized.");

    return NRF_SUCCESS;
}

void motor_control_set(int32_t left_new_speed, int32_t right_new_speed)
{
    left_target_speed = left_new_speed;
    right_target_speed = right_new_speed;
}