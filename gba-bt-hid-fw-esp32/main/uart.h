//
// Created by Shyri on 2019-10-30.
//

#ifndef GBA_BT_HID_FW_ESP32_UART_H
#define GBA_BT_HID_FW_ESP32_UART_H

#include <string.h>

void initUART();

int uartRead(char *message);

void uartWrite(const char *message);

#endif //GBA_BT_HID_FW_ESP32_UART_H
