#include <stdio.h>
#include "multiboot.h"
#include "bluetooth.h"

void app_main(void) {
//    initSPI();
    printf("Initializing bluetooth\n");
    initBluetooth();
    while (1) {
//        printf("Sending Command\n");
//        uint32_t received = send(0xABCDEF91);

//        if (received != 0) {
//            printf("Received: %X\n", received);
//        } else {
//            printf("No Response\n");
//        }
        printf("looping\n");
        ets_delay_us(1000000);
    }
}
