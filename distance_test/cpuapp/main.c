#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/crypto.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

#include <hal/nrf_gpio.h>

#include "mt_peripheral.h"
#include "mt_central.h"
#include "mt_common.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(appcore, 4);

#if ROLE_PERIPHERAL
/**
 * @brief 从机事件处理函数
 */
static void peri_evt_handler(peri_evt_t evt)
{
    switch (evt) {
        case PERI_EVT_DATA_RECV:
            nrf_gpio_pin_toggle(INDICATE_PIN);
            break;

        default:
            break;
    }
}
#else
/**
 * @brief 主机事件处理函数
 */
static void cen_evt_handler(cen_evt_t evt)
{
    switch (evt) {
        case CEN_EVT_DATA_RECV:
            nrf_gpio_pin_toggle(INDICATE_PIN);
            break;

        default:
            break;
    }
}
#endif


void main(void)
{
    int err;
    nrf_gpio_cfg_output(INDICATE_PIN);
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return;
    }
#if ROLE_PERIPHERAL
    mt_peripheral_init();
    mt_start_adv();
    mt_peri_register(peri_evt_handler);
#else
    mt_central_init();
    mt_scan_start();
    mt_central_register(cen_evt_handler);
#endif
}
