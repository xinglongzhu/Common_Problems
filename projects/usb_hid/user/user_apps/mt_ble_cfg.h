
#ifndef MT_BLE_CFG_H__
#define MT_BLE_CFG_H__

/**
 * @brief 获取连接 handle 值
 * 
 * @return 返回连接 handle.
 */
uint16_t mt_get_conn_handle(void);

/**
 * @brief BLE 相关的初始化
 */
void mt_ble_config_init(void);

#endif
