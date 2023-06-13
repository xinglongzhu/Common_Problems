
#ifndef MT_PERIPHERAL_H__
#define MT_PERIPHERAL_H__

/**
 * @brief 从机模块事件类型
 */
typedef enum {
    PERI_EVT_NONE = 0,     /**< 无效事件 */
    PERI_EVT_DATA_RECV,    /**< 从机接收数据事件 */
} peri_evt_t;

/**
 * @brief 从机模块回调函数类型.
 */
typedef void (*peri_evt_hdl_t)(peri_evt_t evt);

/**
 * @brief 向从机模块注册回调函数
 *
 * @param[in] cb 注册到从机模块的回调函数
 **/
void mt_peri_register(peri_evt_hdl_t cb);

/**
 * @brief BLE 从机相关的初始化
 */
void mt_peripheral_init(void);

/**
 * @brief 开启广播
 */
void mt_start_adv(void);

#endif