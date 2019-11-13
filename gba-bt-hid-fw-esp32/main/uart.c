//
// Created by Shyri on 2019-10-30.
//

#include "uart.h"
#include "driver/uart.h"
#include "esp_log.h"


// Note: UART2 default pins IO16, IO17 do not work on ESP32-WROVER module
// because these pins connected to PSRAM
#define ECHO_TEST_TXD   (23)
#define ECHO_TEST_RXD   (25)

// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define ECHO_TEST_RTS   (18)

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS  UART_PIN_NO_CHANGE

#define BUF_SIZE        (127)
#define BAUD_RATE       (115200)

// Read packet timeout
#define PACKET_READ_TICS        (10 / portTICK_RATE_MS)
#define ECHO_TASK_STACK_SIZE    (2048)
#define ECHO_TASK_PRIO          (10)
#define UART_PORT          (UART_NUM_2)

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS  UART_PIN_NO_CHANGE

static const char *TAG = "UART";
uint8_t *data;

void initUART() {
    printf("Initializing UART\n");
//    const int uart_num = ECHO_UART_PORT;

    uart_config_t uart_config = {
            .baud_rate = BAUD_RATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_set_mode(UART_PORT, UART_MODE_RS485_HALF_DUPLEX);

    data = (uint8_t *) malloc(BUF_SIZE);

//    while (1) {
//        //Read data from UART
//        int len = uart_read_bytes(uart_num, data, BUF_SIZE, PACKET_READ_TICS);
//
//        if (len > 0) {
//            printf("Received %u bytes:\n", len);
//
//            char message[len];
//
//            for (int i = 0; i < len; i++) {
//                printf("0x%.2X ", (uint8_t) data[i]);
//                message[i] = data[i];
//                // Add a Newline character if you get a return charater from paste (Paste tests multibyte receipt/buffer)
//                if (data[i] == '\r') {
////                    uart_write_bytes(uart_num, "\n", 1);
//                }
//            }
//
//            if (strncmp((const char *) message, "$$$", len) == 0) {
//                printf("$$$ Received\n", len);
//                uart_write_bytes(uart_num, (const char *) "CMD", strlen("CMD"));
//            }
//
//            printf("\n");
////            uart_write_bytes(uart_num, "]\r\n", 3);
//        } else {
//            // Echo a "." to show we are alive while we wait for input
//            printf("Nothing received\n");
////            uart_write_bytes(uart_num, ".", 1);
//        }
//    }
}

int uartRead(char *message) {
    int len = uart_read_bytes(UART_PORT, data, BUF_SIZE, PACKET_READ_TICS);

    if (len > 0) {
        printf("Received %u bytes:", len);

        for (int i = 0; i < len; i++) {
            message[i] = data[i];
            printf("0x%.2X ", (uint8_t) data[i]);

            // Add a Newline character if you get a return charater from paste (Paste tests multibyte receipt/buffer)
//        if (data[i] == '\r') {
//                    uart_write_bytes(uart_num, "\n", 1);
//        }
        }
        printf("\n");
    }


    return len;
}

void uartWrite(const char *message) {
    printf("Sending %s:\n", message);
    uart_write_bytes(UART_PORT, message, strlen(message));
}