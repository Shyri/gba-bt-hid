#include <stdio.h>
#include "multiboot.h"

void app_main(void) {
    initSPI();

    while (1) {
        printf("Sending Command\n");
        uint32_t received = send(0xABCDEF91);

        if (received != 0) {
            printf("Received: %X\n", received);
        } else {
            printf("No Response\n");
        }
        ets_delay_us(1000000);
    }
}
