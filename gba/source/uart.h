//
// Created by Shyri on 2018-12-31.
//

#ifndef GBA_BT_HID_UART_H
#define GBA_BT_HID_UART_H

#define SIO_CTS           0x0004
#define SIO_PARITY_ODD    0x0008
#define SIO_SEND_DATA     0x0010
#define SIO_RECV_DATA     0x0020
#define SIO_ERROR         0x0040
#define SIO_LENGTH_8      0x0080
#define SIO_USE_FIFO      0x0100
#define SIO_USE_PARITY    0x0200
#define SIO_SEND_ENABLE   0x0400
#define SIO_RECV_ENABLE   0x0800
#define UART_READ_TIMEOUT 50000

void initUART(short baudRate);
//unsigned char uartRead(int timeout);
unsigned char uartRead();
void uartWrite(unsigned char data);
void uartReadMessage(unsigned char * message);
void uartSendMessage(char * message);
#endif //GBA_BT_HID_UART_H
