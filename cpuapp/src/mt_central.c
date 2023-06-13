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
#include <bluetooth/scan.h>
#include <bluetooth/gatt_dm.h>

#include <bluetooth/services/nus.h>
#include <bluetooth/services/nus_client.h>

#include <hal/nrf_gpio.h>

#include "mt_central.h"
#include "mt_common.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(central, 4);

static struct bt_conn *default_conn;
static struct bt_nus_client nus_client;
static cen_evt_hdl_t m_cen_handler = NULL;

/**
 * @brief 服务发现完成处理函数
 */
static void discovery_complete(struct bt_gatt_dm *dm, void *context)
{
    int err;
    struct bt_nus_client *nus = context;
    LOG_INF("Service discovery completed");

    bt_gatt_dm_data_print(dm);

    bt_nus_handles_assign(dm, nus);
    bt_nus_subscribe_receive(nus);

    bt_gatt_dm_data_release(dm);

    err = bt_nus_client_send(&nus_client, "123", 3);
    if (err) {
        LOG_WRN("Failed to send data over BLE connection"
                "(err %d)",
                err);
    }
}

/**
 * @brief 未发现服务处理函数
 */
static void discovery_service_not_found(struct bt_conn *conn, void *context)
{
    LOG_INF("Service not found");
}

/**
 * @brief 服务发现错误处理函数
 */
static void discovery_error(struct bt_conn *conn, int err, void *context)
{
    LOG_WRN("Error while discovering GATT database: (%d)", err);
}

struct bt_gatt_dm_cb discovery_cb = {
    .completed = discovery_complete,
    .service_not_found = discovery_service_not_found,
    .error_found = discovery_error,
};

/**
 * @brief 服务发现处理函数
 */
static void gatt_discover(struct bt_conn *conn)
{
    int err;

    if (conn != default_conn) {
        return;
    }

    err =
        bt_gatt_dm_start(conn, BT_UUID_NUS_SERVICE, &discovery_cb, &nus_client);
    if (err) {
        LOG_ERR("could not start the discovery procedure, error "
                "code: %d",
                err);
    }
}

/**
 * @brief 主机连接处理函数
 */
static void connected(struct bt_conn *conn, uint8_t conn_err)
{
#if !ROLE_PERIPHERAL
    int err;
    struct bt_conn_info info;
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (conn_err) {
        LOG_ERR("Connection failed (err %d)\n", conn_err);
        bt_conn_unref(default_conn);
        default_conn = NULL;
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

    if (conn == default_conn) {
        gatt_discover(conn);
    }

#if !TEST_MOD_CODED
    err = bt_scan_stop();
    if ((!err) && (err != -EALREADY)) {
        LOG_ERR("Stop LE scan failed (err %d)\n", err);
    }   
#endif
     
#endif
}

/**
 * @brief 主机断开连接处理函数
 */
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    int err;
    LOG_INF("Disconnected (reason 0x%02x)\n", reason);

#if !ROLE_PERIPHERAL
    if (default_conn != conn) {
        return;
    }

    bt_conn_unref(default_conn);
    default_conn = NULL;

    err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
    if (err) {
        LOG_ERR("Scanning failed to start (err %d)", err);
    }
#endif
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

/**
 * @brief 主机扫描过滤初始化
 */
static void scan_filter_match(struct bt_scan_device_info *device_info,
                              struct bt_scan_filter_match *filter_match,
                              bool connectable)
{
    int err;
    char addr[BT_ADDR_LE_STR_LEN];
    struct bt_conn_le_create_param *conn_params;

    bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

    LOG_INF("Filters matched. Address: %s connectable: %d", addr, connectable);

#if TEST_MOD_CODED
    err = bt_scan_stop();
    if (err) {
        LOG_ERR("Stop LE scan failed (err %d)\n", err);
    }

    conn_params = BT_CONN_LE_CREATE_PARAM(
        BT_CONN_LE_OPT_CODED | BT_CONN_LE_OPT_NO_1M, BT_GAP_SCAN_FAST_INTERVAL,
        BT_GAP_SCAN_FAST_INTERVAL);

    err = bt_conn_le_create(device_info->recv_info->addr, conn_params,
                            BT_LE_CONN_PARAM_DEFAULT, &default_conn);
    if (err) {
        LOG_ERR("Create conn failed (err %d)\n", err);

        err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
        if (err) {
            LOG_ERR("Scanning failed to start (err %d)\n", err);
            return;
        }
    }

    LOG_INF("Connection pending\n");
#endif
}

static void scan_connecting(struct bt_scan_device_info *device_info,
			    struct bt_conn *conn)
{
	default_conn = bt_conn_ref(conn);
}

#if TEST_MOD_CODED
BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, NULL, NULL);
#else
BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, NULL, scan_connecting);
#endif 

/**
 * @brief 主机扫描初始化
 */
static void scan_init(void)
{
    int err;
#if TEST_MOD_CODED
    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
        .options = BT_LE_SCAN_OPT_CODED | BT_LE_SCAN_OPT_NO_1M
    };

    struct bt_scan_init_param scan_init = { .connect_if_match = 1,
                                            .scan_param = &scan_param,
                                            .conn_param = NULL };
#else
    struct bt_scan_init_param scan_init = {
        .connect_if_match = 1,
    };
#endif
    bt_scan_init(&scan_init);
    bt_scan_cb_register(&scan_cb);

    err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_NUS_SERVICE);
    if (err) {
        LOG_ERR("Scanning filters cannot be set (err %d)\n", err);
        return;
    }

    err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
    if (err) {
        LOG_ERR("Filters cannot be turned on (err %d)\n", err);
    }
}

/**
 * @brief 主机接收数据处理函数
 */
static uint8_t ble_data_received(struct bt_nus_client *nus, const uint8_t *data,
                                 uint16_t len)
{
    ARG_UNUSED(nus);
    cen_evt_t evt = CEN_EVT_DATA_RECV;
    if ((evt != CEN_EVT_NONE) && (NULL != m_cen_handler)) {
        m_cen_handler(evt);
    }
    return BT_GATT_ITER_CONTINUE;
}

/**
 * @brief 客户端初始化
 */
static int nus_client_init(void)
{
    int err;
    struct bt_nus_client_init_param init = { .cb = {
                                                 .received = ble_data_received,
                                                 .sent = NULL,
                                             } };

    err = bt_nus_client_init(&nus_client, &init);
    if (err) {
        LOG_ERR("NUS Client initialization failed (err %d)", err);
        return err;
    }

    LOG_INF("NUS Client module initialized");
    return err;
}

void mt_central_register(cen_evt_hdl_t cb)
{
    m_cen_handler = cb;
}

void mt_scan_start(void)
{
    int err;
    err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
    if (err) {
        LOG_ERR("Scanning failed to start (err %d)\n", err);
        return;
    }
}

void mt_central_init(void)
{
    nus_client_init();
    scan_init();
}
