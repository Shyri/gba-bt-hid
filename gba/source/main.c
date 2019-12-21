#include <gba_console.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <gba.h>
#include "uart.h"
#include "hc05.h"

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
#define DISCONNECT_COUNTER 150

unsigned char xAxis = 0x00;
unsigned char yAxis = 0x00;
unsigned char zAxis = 0x00;
unsigned char rAxis = 0x00;
unsigned char buttons1 = 0x00;
unsigned char buttons2 = 0x00;
uint32_t lastAxis = 0x00000000;
uint16_t lastButtons = 0x0000;
uint32_t disconnectCounter = 0;

typedef enum HC05_BUTTONS {
    BUTTON_A = (1 << 0),
    BUTTON_B = (1 << 1),
    BUTTON_L = (1 << 4),
    BUTTON_R = (1 << 5),
    BUTTON_SELECT = (1 << 4),
    BUTTON_START = (1 << 5),
    PAD_DOWN = 0x7F,
    PAD_UP = 0x80,
    PAD_RIGHT = 0x7F,
    PAD_LEFT = 0x80
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
    printf("\x1b[4;0H    GBA BLUETOOTH GAMEPAD\n");
    printf("\x1b[8;0H                              \n");
    printf("\x1b[9;0H                              \n");
    printf("\x1b[10;0H                              \n");
    printf("\x1b[11;0H                              \n");
    printf("\x1b[12;0H                              \n");
    printf("\x1b[13;0H                              \n");
    printf("\x1b[14;0H                              \n");
}

void sendButtons() {
    if (lastAxis != getCurrentAxises() || lastButtons != getCurrentButtons()) {
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

void enableDiscovery() {
    clearConsole();
    printf("\x1b[9;4HDiscovery Mode Enabled\n");
    printf("\x1b[11;0H Press A to pair last device\n");
    status = DISCOVERING;
}

void autoConnect() {
    clearConsole();
    printf("\x1b[9;9HConnecting...\n");

    if (startCommandMode()) {
        status = CONNECTING;
        connectLast();
    } else {
        enableDiscovery();
        printf("\x1b[14;5HConnection Failed :(\n");
    }
}

void disconnect() {
    status = DISCONNECTING;
    clearConsole();
    printf("\x1b[9;6HDisconnecting...\n");
    sendDisconnect();
    enableDiscovery();
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
        disconnectCounter++;
        if (disconnectCounter == DISCONNECT_COUNTER) {
            disconnect();
            return;
        }
    } else {
        disconnectCounter = 0;
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

//    consoleDemoInit();

    consoleInit(0,        // charbase
                4,        // mapbase
                0,        // background number
                NULL,    // font
                0,        // font size
                15        // 16 color palette
    );

    // set the screen colors, color 0 is the background color
    // the foreground color is index 1 of the selected 16 color palette
    BG_COLORS[0] = RGB8(255, 255, 255);
    BG_COLORS[241] = RGB8(58, 110, 165);

    SetMode(MODE_0 | BG0_ON);

    enableDiscovery();

    initUART(SIO_115200);

    while (1) {
        int keys_pressed;

        VBlankIntrWait();

        resetButtons();

        scanKeys();

        keys_pressed = keysHeld();

        switch (status) {
            case DISCOVERING:
                if (keys_pressed & KEY_A) {
                    autoConnect();
                } else if (checkPaired()) {
                    status = CONNECTED;
                    clearConsole();
                    printf("\x1b[9;10HConnected!\n");
                    printf("\x1b[11;3HHold Start to disconnect\n");
                    break;
                }
                break;
            case CONNECTING:
                if (checkPaired()) {
                    status = CONNECTED;
                    clearConsole();
                    printf("\x1b[9;10HConnected!\n");
                    printf("\x1b[11;3HHold Start to disconnect\n");
                    break;
                }
                break;
            case CONNECTED:
                processButtons(keys_pressed);
                break;
        }
    }
}