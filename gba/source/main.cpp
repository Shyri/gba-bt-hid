#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
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
    printf("\x1b[10;10HHello World!\n");
    printf("\x1b[10;10HBullshit!\n");
    while (1) {
        int keys_pressed, keys_released;

        VBlankIntrWait();

        scanKeys();

        keys_pressed = keysDown();
        keys_released = keysUp();

        if (keys_released & KEY_UP) {
            printf("\x1b[10;10HA\n");
        }
    }
}


