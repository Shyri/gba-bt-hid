//
// Created by Shyri on 2019-10-31.
//

#ifndef GBA_BT_HID_FW_ESP32_HC05_H
#define GBA_BT_HID_FW_ESP32_HC05_H

#include "uart.h"
#include <string.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "nvs.h"

#define STORAGE_NAMESPACE "storage"

void initHC05();

void checkHC05();

#endif //GBA_BT_HID_FW_ESP32_HC05_H
