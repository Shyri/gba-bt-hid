//
// Created by Shyri on 2018-12-31.
//
#include <gba.h>
#include <stdio.h>
#include "uart.h"

bool sending = false;

void serialInterrupt() {
    sending = false;
}

void initUART(short baudRate) {
    REG_RCNT = R_UART; // RCNT Mode Selection, in Normal/Multiplayer/UART modes (R/W)
    REG_SIOCNT = 0; // Reset SIO Control Register
    REG_SIOCNT = baudRate | SIO_LENGTH_8 | SIO_SEND_ENABLE | SIO_RECV_ENABLE | SIO_USE_FIFO | SIO_IRQ | SIO_UART;
    irqSet(IRQ_VBLANK, serialInterrupt);
    irqEnable(IRQ_SERIAL);
}

unsigned char uartRead() {

    // Reset receive data flag
    REG_SIOCNT = REG_SIOCNT | 0x0020;

    // We're using CTS so we must send a LO on the SD terminal to show we're ready
    //REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);

    // Wait until we have a full byte (The recv data flag will go to 0)
    int i = UART_READ_TIMEOUT;
    do {
        i--;
        if(i == 0) {
            return 0x00;
        }
    } while(REG_SIOCNT & 0x0020);

    // Return the character in the data register
    return (unsigned char) REG_SIODATA8;
}

void uartWrite(unsigned char data) {
    // Wait until we have a CTS signal
    while(sending || (REG_RCNT & 0x0010));

    sending = true;
    // Bung our byte into the data register
    REG_SIODATA8 = data;
}


void uartReadMessage(unsigned char * message) {
    int i = 0;
    unsigned char read = uartRead();
    while (read != 0x0D) { // Until we receive a CR
        switch(read) {
            case 0x00: // NULL
                return;
            case 0x0A: // LF
                // Ignore
                break;
            default:
                message[i] = read;
                i++;
        }
        read = uartRead();
    }

    return;
}

void uartSendMessage(char * message) {
    int messageSize = sizeof(message) / sizeof(message[0]);
    for(int i = 0; i < messageSize - 1; i++) {
        uartWrite((unsigned char) message[i]);
    }

    uartWrite(0x0D);
}