/* Copyright (C) Shenzhen MinewSemi Technologies Co., Ltd
   All rights reserved. */

#ifndef _MT_ADC_H
#define _MT_ADC_H
#include <stdint.h>

#define SAMPLES_IN_BUFFER     1      /* adc 采样 buff 大小 */
#define ADC_COLLECR_SMALL_FRE 1000   /* adc 采样周期 ms */


/**
 * @brief adc 采集状态
 */
typedef enum { ADC_COLLECTING, ADC_STOP } adc_statu_t;


/**
 * @brief 开启 adc 采集
 */
void mt_start_adc_collect(void);

/**
 * @brief 关闭 adc 采集
 */
void mt_stop_adc_collect(void);

/**
 * @brief 获取 adc 采集状态
 * 
 * @return 正在采集返回 ADC_COLLECTING 没有采集返回 ADC_STOP
 */
adc_statu_t mt_get_adc_statu(void);

#endif
