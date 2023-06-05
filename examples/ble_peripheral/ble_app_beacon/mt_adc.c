/* Copyright (C) Shenzhen MinewSemi Technologies Co., Ltd
   All rights reserved. */

#include "mt_adc.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_saadc.h"
#include "nrf_saadc.h"
#include "app_timer.h"
#include "app_scheduler.h"

#define NRF_LOG_MODULE_NAME mt_adc
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

APP_TIMER_DEF(m_adc);

/* clang-format off */
static nrf_saadc_value_t m_buffer_pool[SAMPLES_IN_BUFFER];           /* 采集 adc 后存储的位置 */
static adc_statu_t m_is_collecting = ADC_STOP;                       /* adc 采集状态 */
/* clang-format on */

/**
 * @brief 开启 adc 采集.
 */
static void adc_start(void)
{
    if (!nrf_drv_saadc_is_busy()) {
        APP_ERROR_CHECK(nrf_drv_saadc_buffer_convert(m_buffer_pool, SAMPLES_IN_BUFFER));
        nrf_drv_saadc_sample();
    }
}

/**
 * @brief adc 周期采集超时函数.
 */
static void collect_timeout_cb(void *ctx)
{
    UNUSED_PARAMETER(ctx);
    adc_start();   
}

/**
 * @brief adc 采集完成回调函数.
 * 
 * @param[in] p_event adc 事件
 */
static void saadc_callback(nrf_drv_saadc_evt_t const *p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
        m_buffer_pool[0] = m_buffer_pool[0] >= 0 ? m_buffer_pool[0] : 0;
        NRF_LOG_INFO("adc_val=%d",m_buffer_pool[0]);
        nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
    }
}

/**
 * @brief adc 初始化
 */
static void adc_init(void)
{
    APP_ERROR_CHECK(app_timer_create(&m_adc, APP_TIMER_MODE_REPEATED, collect_timeout_cb));
    nrf_drv_saadc_config_t p_config = {
#if (ADC_BIT == 4096)
        .resolution = (nrf_saadc_resolution_t)NRF_SAADC_RESOLUTION_12BIT,
#else
        .resolution = (nrf_saadc_resolution_t)NRF_SAADC_RESOLUTION_10BIT,
#endif
        .oversample         = (nrf_saadc_oversample_t)SAADC_CONFIG_OVERSAMPLE,
        .interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY,
        .low_power_mode     = SAADC_CONFIG_LP_MODE
    };
    nrf_saadc_channel_config_t channe0_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN4);

    APP_ERROR_CHECK(nrf_drv_saadc_init(&p_config, saadc_callback));
    APP_ERROR_CHECK(nrf_drv_saadc_channel_init(0, &channe0_config));

}

/**
 * @brief 关闭 adc
 */
void adc_off(void *p_ctx, uint16_t len)
{
    nrf_drv_saadc_uninit();
}

void mt_start_adc_collect(void)
{
    if (m_is_collecting == ADC_STOP) {    
        NRF_LOG_INFO("mt_start_adc_collect\r");
        m_is_collecting     = ADC_COLLECTING;
        adc_init();
        APP_ERROR_CHECK(app_timer_start(m_adc, APP_TIMER_TICKS(ADC_COLLECR_SMALL_FRE), NULL));
    }
}

void mt_stop_adc_collect(void)
{
    if (m_is_collecting == ADC_COLLECTING) {    
        m_is_collecting = ADC_STOP;
        APP_ERROR_CHECK(app_timer_stop(m_adc));
        APP_ERROR_CHECK(app_sched_event_put(NULL, 0, adc_off));
    }
}

adc_statu_t mt_get_adc_statu(void)
{
    return m_is_collecting;
}
