//
// Created by Shyri on 2018-12-31.
//
#include <gba.h>
#include "uart.h"

bool sending = false;

void serialInterrupt() {
    sending = false;
}

void initUART(short baudRate) {
    REG_RCNT = R_UART; // RCNT Mode Selection, in Normal/Multiplayer/UART modes (R/W)
    REG_SIOCNT = 0; // Reset SIO Control Register
    REG_SIOCNT = baudRate | SIO_LENGTH_8 | SIO_SEND_ENABLE | SIO_RECV_ENABLE | SIO_IRQ | SIO_UART;
    irqSet( IRQ_VBLANK, serialInterrupt);
    irqEnable(IRQ_SERIAL);
}

unsigned char uartRead(void) {
    // Reset receive data flag
    REG_SIOCNT = REG_SIOCNT | 0x0020;

    // We're using CTS so we must send a LO on the SD terminal to show we're ready
    REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);

    // Wait until we have a full byte (The recv data flag will go to 0)
    while(REG_SIOCNT & 0x0020);

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