#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "nrf.h"
#include "app_error.h"
#include "rc_receiver_decoder.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_timer.h"

#define PPI_CHANNELS 6
#define ERR_INTERRUPT_CHANNEL 0xFFFFFFFF
#define TIMING_ADJUSTMENT 1000

static uint32_t rc_gpio_pins[] = {3, 4, 28, 29, 30, 31};
static nrf_timer_task_t rc_timer_capture_tasks[] = {NRF_TIMER_TASK_CAPTURE0, NRF_TIMER_TASK_CAPTURE1, NRF_TIMER_TASK_CAPTURE2, NRF_TIMER_TASK_CAPTURE3, NRF_TIMER_TASK_CAPTURE4, NRF_TIMER_TASK_CAPTURE5};
static nrf_ppi_channel_t ppi_channel_start[PPI_CHANNELS];
static nrf_ppi_channel_t ppi_channel_stop[PPI_CHANNELS];

rc_receiver_event_handler_t rc_event_handler = NULL;

nrfx_timer_t rc_timer = NRF_DRV_TIMER_INSTANCE(3); // Timer 3 and 4 have two additional CC registers which we need.

nrf_drv_gpiote_in_config_t gpio_config_lotohi = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
nrf_drv_gpiote_in_config_t gpio_config_hitolo = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

void nrfx_timer_evt_handler(nrf_timer_event_t event_type, void * p_context)
{
}

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    //NRF_LOG_INFO("Test");
    uint32_t err_code;
    uint32_t interrupt_channel = ERR_INTERRUPT_CHANNEL;
    // Find which rc channel just triggered
    for (uint32_t i = 0; i < PPI_CHANNELS; i++)
    {
        if (pin == rc_gpio_pins[i])
        {
            interrupt_channel = i;
        }
    }
    APP_ERROR_CHECK(interrupt_channel == ERR_INTERRUPT_CHANNEL);

    if (action == NRF_GPIOTE_POLARITY_LOTOHI)
    {
        nrf_drv_gpiote_in_uninit(rc_gpio_pins[interrupt_channel]);
        nrf_drv_gpiote_in_init(rc_gpio_pins[interrupt_channel], &gpio_config_hitolo, nrfx_gpiote_evt_handler);
        nrf_drv_gpiote_in_event_enable(rc_gpio_pins[interrupt_channel], true);

        err_code = nrf_drv_ppi_channel_disable(ppi_channel_start[interrupt_channel]);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_drv_ppi_channel_enable(ppi_channel_stop[interrupt_channel]);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        
        nrf_drv_gpiote_in_uninit(rc_gpio_pins[interrupt_channel]);
        nrf_drv_gpiote_in_init(rc_gpio_pins[interrupt_channel], &gpio_config_lotohi, nrfx_gpiote_evt_handler);
        nrf_drv_gpiote_in_event_enable(rc_gpio_pins[interrupt_channel], true);

        err_code = nrf_drv_ppi_channel_disable(ppi_channel_stop[interrupt_channel]);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_drv_ppi_channel_enable(ppi_channel_start[interrupt_channel]);
        APP_ERROR_CHECK(err_code);
        uint32_t capture_value = nrfx_timer_capture_get(&rc_timer, interrupt_channel);
        //NRF_LOG_INFO("%d %d", interrupt_channel, capture_value/16);
        
        // Translate from timer to a value in the rance [0, 16000>
        // Raw value in range
        if (capture_value >= 16000 && capture_value < 32000)
        {
            rc_event_handler(interrupt_channel, capture_value-16000);
        }
        //Raw value right outside range, assume slight inaccuracy in timing.
        if (capture_value >= 16000 - TIMING_ADJUSTMENT && capture_value < 16000)
        {
            rc_event_handler(interrupt_channel, 0);
        }
        if (capture_value < 32000 + TIMING_ADJUSTMENT && capture_value >= 32000)
        {
            rc_event_handler(interrupt_channel, 15999);
        }
    }
}

void rc_receiver_init(rc_receiver_event_handler_t event_handler)
{
    APP_ERROR_CHECK(event_handler == NULL);
    rc_event_handler = event_handler;
    uint32_t err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
    err_code = nrf_drv_timer_init(&rc_timer, &timer_cfg, nrfx_timer_evt_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_enable(&rc_timer);
    
    for (uint32_t i = 0; i < PPI_CHANNELS; i++)
    {
        nrf_drv_ppi_channel_alloc(&ppi_channel_start[i]);
        nrf_drv_ppi_channel_alloc(&ppi_channel_stop[i]);

        gpio_config_hitolo.pull = NRF_GPIO_PIN_PULLDOWN;
        gpio_config_lotohi.pull = NRF_GPIO_PIN_PULLDOWN;
        nrf_drv_gpiote_in_init(rc_gpio_pins[i], &gpio_config_lotohi, nrfx_gpiote_evt_handler);
        nrf_drv_gpiote_in_event_enable(rc_gpio_pins[i], true);
        err_code = nrf_drv_ppi_channel_assign(ppi_channel_start[i],
                                              nrf_drv_gpiote_in_event_addr_get(rc_gpio_pins[i]),
                                              nrf_drv_timer_task_address_get(&rc_timer,
                                                                             NRF_TIMER_TASK_CLEAR));
        err_code = nrf_drv_ppi_channel_assign(ppi_channel_stop[i],
                                              nrf_drv_gpiote_in_event_addr_get(rc_gpio_pins[i]),
                                              nrf_drv_timer_task_address_get(&rc_timer,
                                                                             rc_timer_capture_tasks[i]));
    }
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_ppi_channel_enable(ppi_channel_start[0]);
    APP_ERROR_CHECK(err_code);
}
