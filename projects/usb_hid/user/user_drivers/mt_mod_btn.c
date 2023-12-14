
#include "mt_mod_btn.h"
#include "app_button.h"
#include "app_timer.h"
#include "mt_module_pin.h"

#define NRF_LOG_MODULE_NAME mt_btn
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BTN_DEBOUNCE_TIME       APP_TIMER_TICKS(20)

static btn_evt_hdl_t    m_btn_handler = NULL;

static void button_handler(uint8_t pin_no, uint8_t button_action)
{
    btn_evt_t evt = BTN_EVT_NONE;

    if (button_action == APP_BUTTON_RELEASE) {
        switch (pin_no) {
            case TEST_PIN:
                evt = BTN_EVT_PUSH;
                break;

            default:
                break;
        }
    }

    if (button_action == APP_BUTTON_PUSH) {
       switch (pin_no) {
           case TEST_PIN:
               evt = BTN_EVT_RELEASE;
               break;

           default:
               break;
       }
    }    

    if((evt != BTN_EVT_NONE) && (NULL != m_btn_handler)) {
        m_btn_handler(evt);
    }
}

void mt_button_register(btn_evt_hdl_t cb)
{
    m_btn_handler = cb;
}

void mt_button_init(void)
{
    static app_button_cfg_t buttons[] = { 
        {TEST_PIN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_handler},
    };

    APP_ERROR_CHECK(app_button_init(
        buttons, sizeof(buttons) / sizeof(buttons[0]), BTN_DEBOUNCE_TIME));
    APP_ERROR_CHECK(app_button_enable());
}