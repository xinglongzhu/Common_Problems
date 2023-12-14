/* Copyright (C) Shenzhen Minew Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_BLE_SERVICE_H__
#define MT_BLE_SERVICE_H__

#include <stdint.h>

/**< ble 发送数据的处理函数类型 */
typedef uint32_t (*ble_tx_data_hdl_t)(uint8_t *p_data, uint16_t len);

/**
 * @brief 根据 role 设置 service 发送数据的函数.
 *
 * @param[in] role 指示当前是主还是从设备.
 */
void mt_ble_serv_tx_hdl_set(void);

/**
 * @brief 发送 hid 数据到对端.
 *
 * @param[in] rep_index report 数组 index.
 * @param[in] p_data    指向要发送的 hid 数据.
 * @param[in] len       要发送的 hid 数据长度.
 */
uint32_t hid_send_report(uint8_t rep_index, uint8_t *p_data, uint16_t len);

/**
 * @brief service 模块初始化.
 */
void ble_service_init(void);

#endif

