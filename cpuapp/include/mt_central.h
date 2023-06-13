

#ifndef MT_CENTRAL_H__
#define MT_CENTRAL_H__

/**
 * @brief 主机模块事件类型
 */
typedef enum {
    CEN_EVT_NONE = 0,     /**< 无效事件 */
    CEN_EVT_DATA_RECV,    /**< 主机接收数据事件 */
} cen_evt_t;

/**
 * @brief 主机模块回调函数类型.
 */
typedef void (*cen_evt_hdl_t)(cen_evt_t evt);

/**
 * @brief 向主机模块注册回调函数
 *
 * @param[in] cb 注册到主机模块的回调函数
 **/
void mt_central_register(cen_evt_hdl_t cb);

/**
 * @brief BLE 主机相关的初始化
 */
void mt_central_init(void);

/**
 * @brief 开启扫描
 */
void mt_scan_start(void);

#endif