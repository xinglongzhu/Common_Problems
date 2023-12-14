#include <string.h>
#include "ble_gap.h"
#include "app_util.h"
#include "app_error.h"
#include "mt_ble_adv.h"

#define NRF_LOG_MODULE_NAME ble_adv
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define ADV_INTERVAL_STEP 100 /**< 广播间隔等级步长, 单位 100ms. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< 广播句柄 */
static uint8_t m_adv_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];     /**< 存放广播数据数组 */
static uint8_t m_rsp_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];     /**< 存放扫描回复数据数组 */
static ble_gap_adv_params_t m_adv_params;                     /**< 广播参数结构体. */

/**< 广播数据信息结构体 */
static ble_gap_adv_data_t m_ble_adv_data = { .adv_data      = { .p_data = m_adv_data },
                                             .scan_rsp_data = { .p_data = m_rsp_data } };

/**
 * @brief 广播参数配置
 */
static void adv_params_config(void)
{
    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    m_adv_params.interval        = MSEC_TO_UNITS(100, UNIT_0_625_MS);
    m_adv_params.p_peer_addr     = NULL;
    m_adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_adv_params.duration        = 0;
    m_adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;

    APP_ERROR_CHECK(sd_ble_gap_adv_set_configure(&m_adv_handle, &m_ble_adv_data, &m_adv_params));

    APP_ERROR_CHECK(
        sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_adv_handle, 0));
}

/**
 * @brief 广播数据组包
 */
static void adv_data_set(void)
{
    uint8_t  idx         = 0;

    m_adv_data[idx++] = 0x02;
    m_adv_data[idx++] = 0x01;
    m_adv_data[idx++] = 0x06;

    m_adv_data[idx++] = 0x03;
    m_adv_data[idx++] = 0x19;
    m_adv_data[idx++] = 0xC1;
    m_adv_data[idx++] = 0x03;    

    m_adv_data[idx++] = 9;
    m_adv_data[idx++] = 0x09;
    m_adv_data[idx++] = 'H';
    m_adv_data[idx++] = 'I';
    m_adv_data[idx++] = 'D';
    m_adv_data[idx++] = '-';    
    m_adv_data[idx++] = 'T';
    m_adv_data[idx++] = 'e';
    m_adv_data[idx++] = 's';
    m_adv_data[idx++] = 't';

    m_ble_adv_data.adv_data.len = idx;
    m_ble_adv_data.scan_rsp_data.len = 0;
}

/**
 * @brief 广播数据设置
 */
static void adv_data_padding(void)
{
    uint32_t err;
    adv_data_set();
    err = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_ble_adv_data, &m_adv_params);
    if (err != NRF_ERROR_INVALID_STATE) {
        APP_ERROR_CHECK(err);
    }
}

void ble_adv_start(void)
{
    adv_params_config();
    adv_data_padding();
    APP_ERROR_CHECK(sd_ble_gap_adv_start(m_adv_handle, 1));
}

void ble_adv_stop(void)
{
    APP_ERROR_CHECK(sd_ble_gap_adv_stop(m_adv_handle));
}
