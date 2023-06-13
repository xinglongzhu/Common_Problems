#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/crypto.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>

#include <bluetooth/services/nus.h>

#include <hal/nrf_gpio.h>

#include "mt_peripheral.h"
#include "mt_common.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(peripheral, 4);

#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

/* 函数声明 */
static void start_advertising_coded(struct k_work *work);
static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
                          uint16_t len);
static void connected(struct bt_conn *conn, uint8_t conn_err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static peri_evt_hdl_t m_peri_handler = NULL;
static struct bt_le_ext_adv *adv;
static K_WORK_DEFINE(start_advertising_worker, start_advertising_coded);
static struct bt_nus_cb nus_cb = {
    .received = bt_receive_cb,
};
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/**
 * @brief 创建 coded 广播
 */
static int create_advertising_coded(void)
{
    int err;
    struct bt_le_adv_param param = BT_LE_ADV_PARAM_INIT(
        BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_EXT_ADV | BT_LE_ADV_OPT_CODED,
        1600, 1600, NULL);

    err = bt_le_ext_adv_create(&param, NULL, &adv);
    if (err) {
        LOG_ERR("Failed to create advertiser set (err %d)\n", err);
        return err;
    }

    err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Failed to set advertising data (err %d)\n", err);
        return err;
    }

    return 0;
}

/**
 * @brief 开启广播
 */
static void start_advertising_coded(struct k_work *work)
{
    int err;

    err = bt_le_ext_adv_start(adv, NULL);
    if (err) {
        LOG_ERR("Failed to start advertising set (err %d)\n", err);
        return;
    }
}

/**
 * @brief 连接处理函数
 */
static void connected(struct bt_conn *conn, uint8_t conn_err)
{
#if ROLE_PERIPHERAL
    int err;
    struct bt_conn_info info;
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (conn_err) {
        LOG_ERR("Connection failed (err %d)\n", conn_err);
        return;
    }

    err = bt_conn_get_info(conn, &info);
    if (err) {
        LOG_ERR("Failed to get connection info (err %d)\n", err);
    } else {
        const struct bt_conn_le_phy_info *phy_info;
        phy_info = info.le.phy;

        LOG_INF("Connected: %s, tx_phy %u, rx_phy %u\n", addr, phy_info->tx_phy,
                phy_info->rx_phy);
    }
#endif
}

/**
 * @brief 断开连接处理函数
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason 0x%02x)\n", reason);
#if (ROLE_PERIPHERAL && TEST_MOD_CODED)
    k_work_submit(&start_advertising_worker);
#endif
}

/**
 * @brief 接收到对端数据处理函数
 */
static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
                          uint16_t len)
{
    peri_evt_t evt = PERI_EVT_DATA_RECV;
    char addr[BT_ADDR_LE_STR_LEN] = { 0 };

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

    if (bt_nus_send(NULL, data, len)) {
        LOG_WRN("Failed to send data over BLE connection");
    }

    if ((evt != PERI_EVT_NONE) && (NULL != m_peri_handler)) {
        m_peri_handler(evt);
    }
}

void mt_peri_register(peri_evt_hdl_t cb)
{
    m_peri_handler = cb;
}

void mt_start_adv(void)
{
#if TEST_MOD_CODED
    k_work_submit(&start_advertising_worker);
#else
    int err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return;
    }
#endif
}

void mt_peripheral_init(void)
{
    int err;
    err = bt_nus_init(&nus_cb);
    if (err) {
        LOG_ERR("Failed to initialize peripheral (err: %d)", err);
        return;
    }
#if TEST_MOD_CODED
    create_advertising_coded();
#endif
}
