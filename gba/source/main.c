#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <gba.h>
#include <string.h>
#include "uart.h"
#include "hc05.h"

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
    BUTTON_SELECT	=	(1<<4),
    BUTTON_START	=	(1<<5),
    PAD_DOWN   = 0x7F,
    PAD_UP   = 0x80,
    PAD_RIGHT   = 0x7F,
    PAD_LEFT   = 0x80
} HC05_BUTTONS_BITS;

typedef enum STATUS {
    DISCOVERING = 0,
    CONNECTING = 1,
    CONNECTED = 2,
    DISCONNECTING = 3,
} STATES;

int status = DISCOVERING;

uint32_t getCurrentAxises() {
    return rAxis | (zAxis << 8) | (yAxis << 16) | (xAxis << 24);
}

uint16_t getCurrentButtons() {
    return buttons2 | (buttons1 << 8);
}

void clearConsole() {
    printf("\x1b[8;1H                    \n");
    printf("\x1b[9;1H                    \n");
    printf("\x1b[10;10H                  \n");
    printf("\x1b[11;10H                  \n");
}

void sendButtons() {
    //printf("\x1b[8;1HLast:  %d\n", lastAxis);
    if(lastAxis != getCurrentAxises() || lastButtons != getCurrentButtons()) {
        sendGamepad(xAxis, yAxis, zAxis, rAxis, buttons1, buttons2);

        lastAxis = getCurrentAxises();
        lastButtons = getCurrentButtons();
    }
}

void resetButtons() {
    xAxis = 0x00;
    yAxis = 0x00;
    zAxis = 0x00;
    rAxis = 0x00;
    buttons1 = 0x00;
    buttons2 = 0x00;
}

void autoConnect() {
    clearConsole();
    printf("\x1b[8;10HConnecting...\n");

    if (startCommandMode()) {
        status = CONNECTING;
        if (connectLast()) {
            status = CONNECTED;
            printf("\x1b[8;10HConnected!\n");
        } else {
            printf("\x1b[8;10HConnection Failed!\n");
            status = DISCOVERING;
        }
    } else {
        status = DISCOVERING;
        printf("\x1b[8;10HConnection Failed!\n");
    }
}

void processButtons(int keys_pressed) {
    if (keys_pressed & KEY_A) {
        buttons1 = buttons1 | BUTTON_A;
    }

    if (keys_pressed & KEY_B) {
        buttons1 = buttons1 | BUTTON_B;
    }

    if (keys_pressed & KEY_L) {
        buttons1 = buttons1 | BUTTON_L;
    }

    if (keys_pressed & KEY_R) {
        buttons1 = buttons1 | BUTTON_R;
    }

    if (keys_pressed & KEY_START) {
        buttons2 = buttons2 | BUTTON_START;
    }

    if (keys_pressed & KEY_SELECT) {
        buttons2 = buttons2 | BUTTON_SELECT;
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

int main(void) {
//---------------------------------------------------------------------------------
    // the vblank interrupt must be enabled for VBlankIntrWait() to work
    // since the default dispatcher handles the bios flags no vblank handler
    // is required
    irqInit();
    irqEnable(IRQ_VBLANK);
    REG_IME = 1;

    consoleDemoInit();

    printf("\x1b[8;1HDiscovery Mode Enabled\n");
    printf("\x1b[10;1HPress A to pair with last device\n");

    initUART(SIO_9600);

    while (1) {
        int keys_pressed;

        VBlankIntrWait();

        resetButtons();

        scanKeys();

        keys_pressed = keysHeld();

        switch (status) {
            case DISCOVERING:
                // TODO Read message to see if paired with device
                if (keys_pressed & KEY_A) {
                    autoConnect();
                }
                break;
            case CONNECTED:
                processButtons(keys_pressed);
                break;
        }
    }
}