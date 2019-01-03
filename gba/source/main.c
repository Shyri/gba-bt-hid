#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <gba.h>
#include "uart.h"

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------

unsigned char xAxis = 0x00;
unsigned char yAxis = 0x00;
unsigned char zAxis = 0x00;
unsigned char rAxis = 0x00;
unsigned char buttons1 = 0x00;
unsigned char buttons2 = 0x00;
uint32_t lastAxis = 0x00000000;
uint16_t lastButtons = 0x0000;

typedef enum HC05_BUTTONS {
    BUTTON_A		=	(1<<0),
    BUTTON_B		=	(1<<1),
    BUTTON_L	=	(1<<4),
    BUTTON_R	=	(1<<5),
    BUTTON_SELECT	=	(1<<6),
    BUTTON_START	=	(1<<7),
    PAD_DOWN   = 0x7F,
    PAD_UP   = 0x80,
    PAD_RIGHT   = 0x7F,
    PAD_LEFT   = 0x80
} HC05_BUTTONS_BITS;

uint32_t getCurrentAxises() {
    return rAxis | (zAxis << 8) | (yAxis << 16) | (xAxis << 24);
}

uint16_t getCurrentButtons() {
    return buttons2 | (buttons1 << 8);
}

void sendHeader(void) {
    uartWrite(0xFD);
    uartWrite(0x06);
}

void sendButtons() {
    if(lastAxis == getCurrentAxises() && lastButtons == getCurrentButtons()) {
        return;
    }

    sendHeader();
    uartWrite(xAxis);
    uartWrite(yAxis);
    uartWrite(zAxis);
    uartWrite(rAxis);
    uartWrite(buttons1);
    uartWrite(buttons2);

    lastAxis = getCurrentAxises();
    lastButtons = getCurrentButtons();
}

void resetButtons() {
    xAxis = 0x00;
    yAxis = 0x00;
    zAxis = 0x00;
    rAxis = 0x00;
    buttons1 = 0x00;
    buttons2 = 0x00;
}

void connect() {
    uartWrite('$');
    uartWrite('$');
    uartWrite('$');
}

int main(void) {
//---------------------------------------------------------------------------------
    // the vblank interrupt must be enabled for VBlankIntrWait() to work
    // since the default dispatcher handles the bios flags no vblank handler
    // is required
    irqInit();
    irqEnable(IRQ_VBLANK);
    REG_IME = 1;

    consoleDemoInit();

    // ansi escape sequence to set print co-ordinates
    // /x1b[line;columnH
    printf("\x1b[10;10HBullshit!\n");

    initUART(SIO_9600);



    while (1) {
        int keys_pressed, keys_released;

        VBlankIntrWait();

        resetButtons();

        scanKeys();

        keys_pressed = keysDown();

        if (keys_pressed & KEY_A) {
            buttons1 = buttons1 | BUTTON_A;
        }

        if (keys_pressed & KEY_B) {
            buttons1 = buttons1 | BUTTON_B;
        }

        if (keys_pressed & KEY_L) {
            buttons2 = buttons1 | BUTTON_L;
        }

        if (keys_pressed & KEY_R) {
            buttons2 = buttons1 | BUTTON_R;
        }


        if (keys_pressed & KEY_START) {
            buttons2 = buttons1 | BUTTON_START;
        }


        if (keys_pressed & KEY_SELECT) {
            buttons2 = buttons1 | BUTTON_SELECT;
        }

        if (keys_pressed & KEY_UP) {
            yAxis = PAD_UP;
        }

        if (keys_pressed & KEY_DOWN) {
            yAxis = PAD_DOWN;
        }

        if (keys_pressed & KEY_RIGHT) {
            xAxis = PAD_RIGHT;
        }

        if (keys_pressed & KEY_LEFT) {
            xAxis = PAD_LEFT;
        }

        sendButtons();
    }
}