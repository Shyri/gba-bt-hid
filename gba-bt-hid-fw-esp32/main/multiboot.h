//
// Created by Shyri on 2019-09-12.
//

#ifndef GBA_BT_HID_FW_SP32_MULTIBOOT_H
#define GBA_BT_HID_FW_SP32_MULTIBOOT_H

#include <stdint.h>
#include <driver/spi_master.h>
#include "gba_rom.h"

void initSPI();

uint32_t send(uint32_t command);

void multiboot();

#endif //GBA_BT_HID_FW_SP32_MULTIBOOT_H
