#include "nrf_delay.h"
#include "app_timer.h"
#include "nrf_drv_wdt.h"
#include "nrfx_wdt.h"
#include "app_error.h"
#include "mt_logic.h"
#include "app_scheduler.h"
#include "nrf_pwr_mgmt.h"
#include "mt_ble_cfg.h"
#include "mt_mod_btn.h"
#include "nrf_gpio.h"
#include "mt_module_pin.h"
#include "mt_serial.h"

#define NRF_LOG_MODULE_NAME main
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

#define SCHED_MAX_EVENT_DATA_SIZE 8  /**< scheduler 事件数据的最大值. */
#define SCHED_QUEUE_SIZE          20 /**< scheduler 队列的长度. */

static void wdt_event_handler(void)
{
    /* todo */
}

/**@brief WDT 初始化.
 */
static void wdt_init(void)
{
    static nrf_drv_wdt_channel_id m_channel_id;
    nrf_drv_wdt_config_t          config = NRF_DRV_WDT_DEAFULT_CONFIG;
    APP_ERROR_CHECK(nrf_drv_wdt_init(&config, wdt_event_handler));
    APP_ERROR_CHECK(nrf_drv_wdt_channel_alloc(&m_channel_id));
}

/**
 * @brief 空闲状态处理
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false) {
        nrf_pwr_mgmt_run();
    }
}

/**
 * @brief 初始化 log 模块
 */
static void log_init(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    log_init();
    APP_ERROR_CHECK(app_timer_init());
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_ERROR_CHECK(nrf_pwr_mgmt_init());
    wdt_init();

    mt_button_init();
    mt_serial_init();
    mt_ble_config_init();

    mt_logic_init();
    nrf_drv_wdt_enable();
    NRF_LOG_INFO("main start\n");
    for (;;) {
        app_sched_execute();
        idle_state_handle();
    }
}