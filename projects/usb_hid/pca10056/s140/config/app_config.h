/* Copyright (C) XingHuoWuXian Technologies Co., Ltd
   All rights reserved. */

#ifndef _CONFIG_H
#define _CONFIG_H

/**
 * @brief 硬件用户配置信息
 */

/**
 * @brief 软件用户配置信息
 */
#define NRF_LOG_ENABLED             1

#if NRF_LOG_ENABLED
#define DEBUG
#define NRF_LOG_BACKEND_RTT_ENABLED      1
#define NRF_LOG_BACKEND_SERIAL_USES_RTT  1
#endif

#endif
