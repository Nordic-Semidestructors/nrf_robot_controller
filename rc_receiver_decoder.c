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

#define RC_GPIO_PIN_0 3
#define RC_GPIO_PIN_0 4
#define RC_GPIO_PIN_0 28
#define RC_GPIO_PIN_0 29
#define RC_GPIO_PIN_0 30
#define RC_GPIO_PIN_0 31

static nrf_ppi_channel_t ppi_channel0;
static nrf_ppi_channel_t ppi_channel1;
static nrf_ppi_channel_t ppi_channel2;
static nrf_ppi_channel_t ppi_channel3;
static nrf_ppi_channel_t ppi_channel4;
static nrf_ppi_channel_t ppi_channel5;


nrf_drv_gpiote_in_config_t gpio_0_config_lotohi = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
nrf_drv_gpiote_in_config_t gpio_0_config_hitolo = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

void nrfx_gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (action == NRF_GPIOTE_POLARITY_LOTOHI)
    {
        NRF_LOG_INFO("hit lo to hi");
        nrf_drv_gpiote_in_uninit(RC_GPIO_PIN_0);
        nrf_drv_gpiote_in_init(RC_GPIO_PIN_0, &gpio_0_config_hitolo, nrfx_gpiote_evt_handler);
        nrf_drv_gpiote_in_event_enable(RC_GPIO_PIN_0, true);
    }
    else
    {
        NRF_LOG_INFO("hit hi to lo");
        nrf_drv_gpiote_in_uninit(RC_GPIO_PIN_0);
        nrf_drv_gpiote_in_init(RC_GPIO_PIN_0, &gpio_0_config_lotohi, nrfx_gpiote_evt_handler);
        nrf_drv_gpiote_in_event_enable(RC_GPIO_PIN_0, true);
    }
    
}

void rc_receiver_init()
{
    uint32_t err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_ppi_channel_alloc(&ppi_channel0);

    gpio_0_config_hitolo.pull = NRF_GPIO_PIN_PULLDOWN;
    gpio_0_config_lotohi.pull = NRF_GPIO_PIN_PULLDOWN;
    nrf_drv_gpiote_in_init(RC_GPIO_PIN_0, &gpio_0_config_lotohi, nrfx_gpiote_evt_handler);
    nrf_drv_gpiote_in_event_enable(RC_GPIO_PIN_0, true);
}
