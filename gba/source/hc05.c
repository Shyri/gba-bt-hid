#include <gba.h>
#include <string.h>
#include "uart.h"


bool prefix(const char *pre, const char *str){
    return strncmp(pre, str, strlen(pre)) == 0;
}

void sendHeader() {
    uartWrite(0xFD);
    uartWrite(0x06);
}

bool startCommandMode() {
    uartWrite('$');
    uartWrite('$');
    uartWrite('$');

    unsigned char message[50];
    uartReadMessage(message);

    return strcmp("CMD", message) == 0;
}

bool connectLast() {
    uartSendMessage("CFR");

    unsigned char message[50];
    uartReadMessage(message);

    return prefix("%CONNECT", message);
}

void sendGamepad(int xAxis, int yAxis, int zAxis, int rAxis, int buttons1, int buttons2) {
    sendHeader();
    uartWrite(xAxis);
    uartWrite(yAxis);
    uartWrite(zAxis);
    uartWrite(rAxis);
    uartWrite(buttons1);
    uartWrite(buttons2);
}

bool checkPaired() {
    unsigned char message[50];
    uartReadMessage(message);

    return prefix("%CONNECT", message);
}

void sendDisconnect() {
    uartWrite(0x00);
    uartWrite(0x0D);
}