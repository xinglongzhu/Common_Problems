
#include "nrf.h"
#include "nrf_gpio.h"
#include "nordic_common.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_drv_wdt.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "mt_logic.h"
#include "mt_mod_btn.h"
#include "mt_module_pin.h"
#include "mt_serial.h"
#include "mt_ble_adv.h"

#define NRF_LOG_MODULE_NAME logic
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define WDT_FEED_TIME APP_TIMER_TICKS(4000) /**< 看门狗喂狗间隔 */

APP_TIMER_DEF(m_wdt_timer); /**< 看门狗喂狗定时器. */

/**
 * @brief 看门狗喂狗
 */
static void wdt_feed_sched_handler(void *p_ctx, uint16_t len)
{
    nrf_drv_wdt_feed();
    nrf_delay_us(150);
}

/**
 * @brief 看门狗喂狗定时器超时处理函数.
 */
static void wdt_timeout_handler(void *p_context)
{
    APP_ERROR_CHECK(app_sched_event_put(NULL, 0, wdt_feed_sched_handler));
}

/**
 * @brief 串口模块上报的事件处理函数.
 *
 * @param[in] p_evt 指向串口模块上报的事件.
 */
static void serial_evt_handler(serial_evt_t *p_evt)
{
    ASSERT(p_evt != NULL);

    switch (p_evt->evt_id) {
        case SERIAL_EVT_RX_OVER:
            NRF_LOG_HEXDUMP_INFO(p_evt->p_data, p_evt->len);
            break;

        case SERIAL_EVT_TX_COMPLETE:
            NRF_LOG_INFO("SERIAL_EVT_TX_COMPLETE");
            break;

        default:
            break;
    }
}

void mt_logic_init(void)
{
    mt_serial_hdl_register(serial_evt_handler);
    mt_serial_ctrl_handler(true);

    APP_ERROR_CHECK(app_timer_create(&m_wdt_timer, APP_TIMER_MODE_REPEATED, wdt_timeout_handler));
    APP_ERROR_CHECK(app_timer_start(m_wdt_timer, WDT_FEED_TIME, NULL));
    ble_adv_start();
}