/* Copyright (C) Shenzhen MinewSemi Technologies Co., Ltd 
   All rights reserved. */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "nrf_gpio.h"
#include "app_timer.h"
#include "nordic_common.h"
#include "nrf_error.h"
#include "app_error.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define WAIT_FOR( m ) do { while (!m); m = 0; } while(0)

/**
 * @brief 时钟模块初始化.
 */
static void clock_init(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {}

    NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);

    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {}
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

static void rssi_measurer_configure_radio(void)
{
    NRF_RADIO->POWER  = 1;
    NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk | RADIO_SHORTS_END_DISABLE_Msk;
    NVIC_EnableIRQ(RADIO_IRQn);
}

/**
 * @brief 设置扫描频率 2400 + frequency(MHz).
 */
static uint8_t rssi_measurer_scan_frequency(uint8_t frequency)
{
    uint8_t sample = 0;

    NRF_RADIO->FREQUENCY  = frequency;
    NRF_RADIO->TASKS_RXEN = 1;

    WAIT_FOR(NRF_RADIO->EVENTS_READY);
    NRF_RADIO->TASKS_RSSISTART = 1;
    WAIT_FOR(NRF_RADIO->EVENTS_RSSIEND);

    sample = 127 & NRF_RADIO->RSSISAMPLE;

    NRF_RADIO->TASKS_DISABLE = 1;
    WAIT_FOR(NRF_RADIO->EVENTS_DISABLED);
    return sample;
}

/**
 * @brief 获取各个频段的 rssi .
 */
static void get_channel_rssi(void)
{
    uint8_t rssi = 0;

    for(uint8_t i = 0; i < 80; i++){
      rssi = rssi_measurer_scan_frequency(i);
      NRF_LOG_INFO("frequency:%d rssi:-%d ",2400+i,rssi);
      NRF_LOG_PROCESS();
    }
}

int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    clock_init();

    rssi_measurer_configure_radio();

    get_channel_rssi();

    while (true)
    {
        NRF_LOG_PROCESS();
    }
}

/**
 *@}
 **/
