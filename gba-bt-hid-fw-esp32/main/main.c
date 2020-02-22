//
// Created by Shyri on 2019-10-22.
//
#include "multiboot.h"
#include "hc05.h"

#define __BTSTACK_FILE__ "main.c"

int btstack_main(int argc, const char *argv[]);

int btstack_main(int argc, const char *argv[]) {
    multiboot();
    initHC05();

    return 0;
}