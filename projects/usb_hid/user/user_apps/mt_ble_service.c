#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "mt_ble_service.h"
#include "ble_hids.h"
#include "mt_hid_config.h"
#include "mt_ble_cfg.h"
#include "app_error.h"

#define NRF_LOG_MODULE_NAME ble_srv
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_TUNNEL_FIFO_SIZE 1024 /**< tunnel_fifo 的长度. */

static bool m_in_boot_mode = false; /**< 当前协议模式. */

NRF_BLE_QWR_DEF(m_qwr);
BLE_HIDS_DEF(m_hids, NRF_SDH_BLE_TOTAL_LINK_COUNT, 3);

/**
 * @brief 处理 Queue_Write 模块的错误
 *
 * @param[in] nrf_error 模块出现的具体的错误码
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void on_hids_evt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt)
{
    switch (p_evt->evt_type) {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            m_in_boot_mode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            m_in_boot_mode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            break;

        default:
            break;
    }
}

static void hid_service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

uint32_t hid_send_report(uint8_t rep_index, uint8_t *p_data, uint16_t len)
{
    ret_code_t err_code;

    uint16_t conn_handle = mt_get_conn_handle();
    NRF_LOG_INFO("m_in_boot_mode=%d", m_in_boot_mode);
    if (!m_in_boot_mode) {
        err_code = ble_hids_inp_rep_send(&m_hids, rep_index, len, p_data, conn_handle);
    } else {
        err_code = ble_hids_boot_kb_inp_rep_send(&m_hids, len, p_data, conn_handle);
    }

    return err_code;
}

static void hids_init(void)
{
    nrf_ble_qwr_init_t qwr_init = { 0 };
    qwr_init.error_handler      = nrf_qwr_error_handler;

    ret_code_t                err_code;
    ble_hids_init_t           hids_init_obj;
    ble_hids_inp_rep_init_t * p_input_report;
    ble_hids_outp_rep_init_t *p_output_report;

    uint8_t i = 0;

    static ble_hids_inp_rep_init_t  input_report_array[INPUT_REP_COUNT];
    static ble_hids_outp_rep_init_t output_report_array[OUTPUT_REP_COUNT];

    static const hid_report_t m_input_report_info[INPUT_REP_COUNT] = {
        { INPUT_REP_KEYBOARD_ID, 6 },
        { INPUT_REP_MOUSE_ID, 4 },
    };
    static const hid_report_t m_output_report_info[OUTPUT_REP_COUNT] = { { OUTPUT_REP_KEYBOARD_ID,
                                                                           1 } };
    /* clang-format off */
    static uint8_t report_map_data[] = {
        #include "mt_hid_keyboard.h"
        #include "mt_hid_mouse.h"
    };
    /* clang-format on */

    APP_ERROR_CHECK(nrf_ble_qwr_init(&m_qwr, &qwr_init));

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

    for (i = 0; i < INPUT_REP_COUNT; i++) {
        p_input_report                      = &input_report_array[i];
        p_input_report->max_len             = m_input_report_info[i].report_len;
        p_input_report->rep_ref.report_id   = m_input_report_info[i].report_id;
        p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

        p_input_report->sec.cccd_wr = SEC_JUST_WORKS;
        p_input_report->sec.wr      = SEC_JUST_WORKS;
        p_input_report->sec.rd      = SEC_JUST_WORKS;

        NRF_LOG_INFO("m_input_report_info[%d].report_i=%d", i, m_input_report_info[i].report_id);
    }

    for (i = 0; i < OUTPUT_REP_COUNT; i++) {
        p_output_report                      = &output_report_array[i];
        p_output_report->max_len             = m_output_report_info[i].report_len;
        p_output_report->rep_ref.report_id   = m_output_report_info[i].report_id;
        p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

        p_output_report->sec.wr = SEC_JUST_WORKS;
        p_output_report->sec.rd = SEC_JUST_WORKS;
        NRF_LOG_INFO("m_output_report_info[%d].report_i=%d", i, m_output_report_info[i].report_id);
    }

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = hid_service_error_handler;
    hids_init_obj.is_kb                          = true;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = INPUT_REP_COUNT;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = OUTPUT_REP_COUNT;
    hids_init_obj.p_outp_rep_array               = output_report_array;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = NULL;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags =
        HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;
    hids_init_obj.included_services_count   = 0;
    hids_init_obj.p_included_services_array = NULL;

    hids_init_obj.rep_map.rd_sec         = SEC_JUST_WORKS;
    hids_init_obj.hid_information.rd_sec = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_inp_rep_sec.cccd_wr = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_inp_rep_sec.rd      = SEC_JUST_WORKS;

    hids_init_obj.boot_kb_outp_rep_sec.rd = SEC_JUST_WORKS;
    hids_init_obj.boot_kb_outp_rep_sec.wr = SEC_JUST_WORKS;

    hids_init_obj.protocol_mode_rd_sec = SEC_JUST_WORKS;
    hids_init_obj.protocol_mode_wr_sec = SEC_JUST_WORKS;
    hids_init_obj.ctrl_point_wr_sec    = SEC_JUST_WORKS;

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}

void ble_service_init(void)
{
    hids_init();
}
