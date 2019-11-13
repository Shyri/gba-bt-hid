//
// Created by Shyri on 2019-09-12.
//

#include <stdint.h>
#include <esp_err.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <string.h>
#include "multiboot.h"

#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  19
#define PIN_NUM_CS   22

#define PIN_NUM_DC   21
#define PIN_NUM_RST  18
#define PIN_NUM_BCKL 5

const uint8_t gba_header[] = {
        0x2E, 0x00, 0x00, 0xEA, 0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
        0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19,
        0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33,
        0x82, 0xE3, 0xCE, 0xBF, 0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
        0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27,
        0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00,
        0x40, 0xA7, 0x0E, 0xFD, 0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
        0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF,
        0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63,
        0x87, 0xF0, 0x3C, 0xAF, 0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x30, 0x31, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00};

void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
    int dc = (int) t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

spi_device_handle_t spi;
uint32_t fcnt;

void initSPI() {
    esp_err_t ret;

    spi_bus_config_t buscfg = {
            .miso_io_num=PIN_NUM_MISO,
            .mosi_io_num=PIN_NUM_MOSI,
            .sclk_io_num=PIN_NUM_CLK,
            .quadwp_io_num=-1,
            .quadhd_io_num=-1,
            .max_transfer_sz=8
    };

    spi_device_interface_config_t devcfg = {
            .clock_speed_hz=5 * 100 * 1000,           //Clock out at 500 Khz
            .mode=3,                                //SPI mode 3
            .spics_io_num=PIN_NUM_CS,               //CS pin
            .queue_size=7,                          //We want to be able to queue 7 transactions at a time
            .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };

    //Initialize the SPI bus
    ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);

    //Attach the device to the SPI bus
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}

void shutdownSPI() {
    spi_bus_remove_device(spi);
    spi_bus_free(HSPI_HOST);
}

uint32_t send(uint32_t command) {
    uint8_t tx[4];
    tx[0] = (uint8_t)((command >> 24) & 0xFF);
    tx[1] = (uint8_t)((command >> 16) & 0xFF);
    tx[2] = (uint8_t)((command >> 8) & 0xFF);
    tx[3] = (uint8_t)(command & 0xFF);


    esp_err_t ret;
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(spi_transaction_t));
    trans.length = 8 * 4;
    trans.user = (void *) 1;
    trans.tx_buffer = tx;
    trans.flags = SPI_TRANS_USE_RXDATA;
    ret = spi_device_transmit(spi, &trans);
    ESP_ERROR_CHECK(ret); // TODO Error check

    return trans.rx_data[3] | (trans.rx_data[2] << 8) | (trans.rx_data[1] << 16) | (trans.rx_data[0] << 24);
}

void sendHandshake(uint32_t rv) {
    // handshake_data 11h+client_data[1]+client_data[2]+client_data[3]
    uint32_t handshake_data = (((rv >> 16) + 0xf) & 0xff) | 0x00006400;
    printf("Sending Handshake: %X", handshake_data);
    rv = send(handshake_data);
    printf("Response: %X", rv);
}

void sendROMHeader() {
    printf("Sending ROM Header");
    uint32_t block;
    memset(&block, 0, sizeof(uint32_t));
    // Send Header in blocks of two bytes
    for (uint32_t i = 0; i <= 0x5f; i++) {
        block = gba_header[2 * i];
        block = gba_header[2 * i + 1] << 8 | block;
        fcnt += 2;

        send(block);
        printf("%X", block);
    }
    printf("Success");
    uint32_t rv = send(0x00006200); // Transfer of header data completed
    printf("%X", rv);
}

void multiboot() {
    initSPI();

    long romLength = sizeof(rom);
    uint32_t rom_size = sizeof(gba_header) + romLength;
    rom_size = (rom_size + 0xf) & 0xFFFFFFF0; // Align rom length to 16
    printf("ROM Size:\n");
    printf("%X\n", rom_size);

    uint32_t rv;
    memset(&rv, 0, sizeof(uint32_t));

    printf("Waiting for GBA\n");
    while (rv != 0x72026202) {
        rv = send(0x00006202);
    }
    printf("GBA Found: \n");
    printf("%X\n", rv);

    rv = 0;

    send(0x00006202); // Found GBA
    send(0x00006102); // Recognition OK

    sendROMHeader();  // Transfer C0h bytes header data in units of 16bits with no encrpytion

    rv = send(0x00006202); // Exchange master/slave info again
    printf("%X\n", rv);

    printf("Sending Palette\n");
    // palette_data as "81h+color*10h+direction*8+speed*2", or as "0f1h+color*2" for fixed palette, whereas color=0..6, speed=0..3, direction=0..1.
    // Then wait until 0x73hh**** is received. hh represents client_data
    while (((rv >> 24) & 0xFF) != 0x73) {
        rv = send(0x000063D1);
        printf("%X\n", rv);
    }

    uint32_t client_data = ((rv >> 16) & 0xFF); // Random client generated data used for later handshake
    printf("Client Data:");
    printf("%X\n", client_data);

    uint32_t m = ((rv & 0x00ff0000) >> 8) + 0xffff00d1;
    uint32_t h = ((rv & 0x00ff0000) >> 16) + 0xf;

    sendHandshake(rv);

    printf("Sending length information: ");
    printf("%X\n", (rom_size - 0x190) / 4);
    rv = send((rom_size - 0x190) / 4); // Send length information and receive random data[1-3] (seed)
    printf("%X\n", rv);

    uint32_t f = (((rv & 0x00ff0000) >> 8) + h) | 0xffff0000;
    uint32_t c = 0x0000c387;

    uint32_t bytes_sent = 0;
    uint32_t w, w2, bitt;
    int i = 0;

    printf("Sending ROM\n");
    while (fcnt < rom_size) {
        if (bytes_sent == 32) {
            bytes_sent = 0;
        }

        w = rom[i] | (rom[i + 1] << 8) | (rom[i + 2] << 16) | (rom[i + 3] << 24);

        i = i + 4;
        bytes_sent += 4;

        if (fcnt % 0x80 == 0 || fcnt > 63488 || fcnt == rom_size) {
            printf("%X", fcnt);
            printf("/");
            printf("%X\n", rom_size);
        }


        w2 = w;

        for (bitt = 0; bitt < 32; bitt++) {
            if ((c ^ w) & 0x01) {
                c = (c >> 1) ^ 0x0000c37b;
            } else {
                c = c >> 1;
            }
            w = w >> 1;
        }


        m = (0x6f646573 * m) + 1;
        rv = send(w2 ^ ((~(0x02000000 + fcnt)) + 1) ^ m ^ 0x43202f2f);

        fcnt = fcnt + 4;
    }
    printf("%X\n", rv);
    printf("ROM sent! Doing checksum now...\ne");


    for (bitt = 0; bitt < 32; bitt++) {
        if ((c ^ f) & 0x01) {
            c = (c >> 1) ^ 0x0000c37b;
        } else {
            c = c >> 1;
        }

        f = f >> 1;
    }
    printf("CRC: ");
    printf("%X\n", c);

    printf("Waiting for CRC\n");
    while (rv != 0x00750065) {
        rv = send(0x00000065);
    }


    rv = send(0x00000066);
    printf("%X\n", rv);

    printf("Exchanging CRC\n");
    rv = send(c);
    printf("%X\n", rv);

    printf("Done!\n");


    // ENABLE UART
//    digitalWrite(MB_PIN, LOW);
//    digitalWrite(UART_PIN, HIGH);
    shutdownSPI();
    printf("SPI shut down\n");
}