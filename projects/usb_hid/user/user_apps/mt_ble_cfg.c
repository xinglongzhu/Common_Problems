
#include <string.h>
#include "ble.h"
#include "nrf_sdh.h"
#include "nrf_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_gatt.h"
#include "ble_gap.h"
#include "ble_conn_params.h"
#include "app_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "mt_ble_cfg.h"
#include "mt_ble_service.h"
#include "mt_ble_adv.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"

#define NRF_LOG_MODULE_NAME ble_cfg
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define SEC_PARAM_BOND            1
#define SEC_PARAM_MITM            0
#define SEC_PARAM_LESC            0
#define SEC_PARAM_KEYPRESS        0
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE
#define SEC_PARAM_OOB             0
#define SEC_PARAM_MIN_KEY_SIZE    7
#define SEC_PARAM_MAX_KEY_SIZE    16

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< 连接句柄. */
NRF_BLE_GATT_DEF(m_gatt);

/**
 * @brief 处理来自协议栈的BLE事件回调函数.
 *
 * @param[in] p_ble_evt BLE 协议栈事件
 * @param[in] p_context 未使用
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ble_gap_evt_t const *p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id) {

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            ble_adv_start();
            NRF_LOG_INFO("Disconnected.");
            break;

        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            NRF_LOG_INFO("Connected.");
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            break;

        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            NRF_LOG_INFO("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x "
                         "kdist_peer:0x%x",
                         p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                         p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                         p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                         *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                         *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
            break;            

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            APP_ERROR_CHECK(sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0));
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys = {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            APP_ERROR_CHECK(sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys));
        } break;

        case BLE_GAP_EVT_PHY_UPDATE: {
            ble_gap_evt_phy_update_t const *p_phy_evt = &p_ble_evt->evt.gap_evt.params.phy_update;
            NRF_LOG_INFO("PHY update tx %d rx %d", p_phy_evt->tx_phy, p_phy_evt->rx_phy);
        } break;

        case BLE_GAP_EVT_TIMEOUT:
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
                NRF_LOG_INFO("Connection timed out.");
            }
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            APP_ERROR_CHECK(sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                                  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            APP_ERROR_CHECK(sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                                  BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION));
            break;

        default:
            break;
    }
}

/**
 * @brief 用于配置连接参数更新的具体参数, 设备名称
 */
static void gap_params_init(void)
{
    uint8_t                 adv_name[] = "HID-Test";
    ble_gap_conn_params_t   conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    APP_ERROR_CHECK(sd_ble_gap_device_name_set(&sec_mode, adv_name, 8));

    APP_ERROR_CHECK(sd_ble_gap_appearance_set(BLE_APPEARANCE_HID_KEYBOARD));
    memset(&conn_params, 0, sizeof(conn_params));

    conn_params.min_conn_interval = MSEC_TO_UNITS(20, UNIT_1_25_MS);
    conn_params.max_conn_interval = MSEC_TO_UNITS(40, UNIT_1_25_MS);

    conn_params.conn_sup_timeout = MSEC_TO_UNITS(4000, UNIT_10_MS);

    APP_ERROR_CHECK(sd_ble_gap_ppcp_set(&conn_params));
}

/**
 * @brief 连接参数模块事件处理.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt)
{
    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        NRF_LOG_WARNING("Conn params evt failed dinconnect!!!");
        APP_ERROR_CHECK(sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE));
    }
}

/**@brief 连接参数错误处理函数
 *
 * @param[in] nrf_error 连接参数错误值.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    if (nrf_error != NRF_ERROR_INVALID_STATE) /* ignore */
        APP_ERROR_HANDLER(nrf_error);
    else {
        NRF_LOG_WARNING("Conn param invaild state disconnect!!!");
        sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
    }
}

/**
 * @brief 连接参数模块初始化
 */
static void conn_params_init(void)
{
    ble_conn_params_init_t cp_init;
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = APP_TIMER_TICKS(100);
    cp_init.next_conn_params_update_delay  = APP_TIMER_TICKS(5000);
    cp_init.max_conn_params_update_count   = 3;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    APP_ERROR_CHECK(ble_conn_params_init(&cp_init));
}

/**
 * @brief gatt 事件处理.
 */
static void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        NRF_LOG_INFO("MTU update %d", p_evt->params.att_mtu_effective - 3);
    }
}
/**
 * @brief gatt 初始化, 设置 MTU 长度.
 */
static void gatt_init(void)
{
    APP_ERROR_CHECK(nrf_ble_gatt_init(&m_gatt, gatt_evt_handler));
    APP_ERROR_CHECK(nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE));
}

/**
 * @brief 初始化协议栈.
 */
static void ble_stack_init()
{
    ret_code_t err_code;
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    uint32_t ram_start = 0;
    err_code           = nrf_sdh_ble_default_cfg_set(1, &ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    NRF_SDH_BLE_OBSERVER(m_ble_observer, 2, ble_evt_handler, NULL);
    APP_ERROR_CHECK(sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE));
}

/**
 * @brief peer manager 事件处理函数.
 * 
 * @param[in] p_evt 事件
 */
static void pm_evt_handler(pm_evt_t const *p_evt)
{
    if (p_evt->evt_id == PM_EVT_CONN_SEC_CONFIG_REQ) {
        pm_conn_sec_config_t conn_sec_config = { .allow_repairing = true };
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
    } else if (p_evt->evt_id == PM_EVT_CONN_SEC_SUCCEEDED) {
        NRF_LOG_INFO("PM_EVT_CONN_SEC_SUCCEEDED");
    } else if (p_evt->evt_id == PM_EVT_CONN_SEC_START) {
        NRF_LOG_INFO("PM_EVT_CONN_SEC_START");
    }
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);
}

/**
 * @brief peer_manager 初始化
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;

    APP_ERROR_CHECK(pm_init());

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    APP_ERROR_CHECK(pm_sec_params_set(&sec_param));
    APP_ERROR_CHECK(pm_register(pm_evt_handler));
}

uint16_t mt_get_conn_handle(void)
{
    return m_conn_handle;
}

void mt_ble_config_init(void)
{
    ble_stack_init();
    peer_manager_init();
    gap_params_init();
    conn_params_init();
    gatt_init();
    ble_service_init();
}
