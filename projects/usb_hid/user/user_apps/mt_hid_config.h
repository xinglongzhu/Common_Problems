/* Copyright (C) Shenzhen MinewSemi Technologies Co., Ltd
   All rights reserved. */

#ifndef MT_HID_CFG_H__
#define MT_HID_CFG_H__

#include "stdint.h"

#define BASE_USB_HID_SPEC_VERSION  0x0101

#define INPUT_REP_KEYBOARD_ID  1
#define INPUT_REP_MOUSE_ID     2
#define OUTPUT_REP_KEYBOARD_ID 0

#define INPUT_REP_COUNT  2
#define OUTPUT_REP_COUNT 1

/**
 * @brief 定义 hid report 结构体数据类型 
 */
typedef struct {
    uint8_t report_id;
    uint8_t report_len;
} hid_report_t;

#endif
