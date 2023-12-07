/* Copyright (C) XingHuoWuXian Technologies Co., Ltd
   All rights reserved. */

#define NRF_LOG_MODULE_NAME main
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
NRF_LOG_MODULE_REGISTER();

/**
 * @brief 空闲状态处理
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false) {}
}

/**
 * @brief 初始化 log 模块
 */
static void log_init(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main(void)
{
    log_init();
    NRF_LOG_INFO("main start\n");
    for (;;) { idle_state_handle(); }
}