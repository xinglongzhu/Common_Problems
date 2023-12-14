
#ifndef MT_MOD_BTN_H__
#define MT_MOD_BTN_H__

/**
 * @brief 按键模块事件类型
 */
typedef enum {
    BTN_EVT_NONE = 0,       /**< 无效事件 */
    BTN_EVT_PUSH,           /**< 按键按下事件 */
    BTN_EVT_RELEASE,        /**< 按键松开事件 */    
} btn_evt_t;

/**
 * @brief 按键模块回调函数类型.
 */
typedef void (*btn_evt_hdl_t)(btn_evt_t evt);

/**
 * @brief sx126x gpio 回调注册函数.
 */
typedef void (*hgpio_callback_t)( void* context );

/**
 * @brief 向 button 注册回调函数
 *
 * @param[in] callbacks 注册到 button 的回调函数
 **/
void hgpio_register_callback(hgpio_callback_t callbacks);

/**
 * @brief 向 button 注册回调函数
 *
 * @param[in] cb 注册到 button 的回调函数
 **/
void mt_button_register(btn_evt_hdl_t cb);

/**
 * @brief 初始化 button
 **/
void mt_button_init(void);

#endif