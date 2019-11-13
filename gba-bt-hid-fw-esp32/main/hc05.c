//
// Created by Shyri
// Simulates a subset of HC-05 Bluetooth module commands and operations
//
#include "hc05.h"

#include "btstack.h"
#include "btstack_event.h"
#include "btstack_stdin.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "btstack_run_loop_freertos.h"
#include "driver/gpio.h"

#define DEVICE_NAME "GBA Bluetooth"

#define COM_MODE_REQUEST "$$$"
#define COM_MODE_RESPONSE "CMD"
#define AUTOCONNECT_REQUEST "CFR"
#define CONNECTED_RESULT "%CONNECT"


static bool inCommandMode = false;

typedef enum DESCRIPTOR_BUTTONS {
    DPAD_RELEASED = 0x08,
    DPAD_UP = 0x00,
    DPAD_DOWN = 0x04,
    DPAD_LEFT = 0x06,
    DPAD_RIGHT = 0x02,

    HID_KEY_A = 0x40,
    HID_KEY_B = 0x20,

    HID_KEY_R = 0x08,
    HID_KEY_L = 0x04,

    HID_KEY_START = 0x20,
    HID_KEY_SELECT = 0x10,

} DESCRIPTOR_BUTTONs_BITS;

typedef enum HC05_BUTTONS {
    BUTTON_A = (1 << 0),
    BUTTON_B = (1 << 1),
    BUTTON_L = (1 << 4),
    BUTTON_R = (1 << 5),
    BUTTON_SELECT = (1 << 4),
    BUTTON_START = (1 << 5),
    PAD_DOWN = 0x7F,
    PAD_UP = 0x80,
    PAD_RIGHT = 0x7F,
    PAD_LEFT = 0x80
} HC05_BUTTONS_BITS;

const uint8_t hid_descriptor_gamecube[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x01,        // Collection (Application)
        //Padding
        0x95, 0x03,          //     REPORT_COUNT = 3
        0x75, 0x08,          //     REPORT_SIZE = 8
        0x81, 0x03,          //     INPUT = Cnst,Var,Abs
        //DPAD
        0x09, 0x39,        //   Usage (Hat switch)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x07,        //   Logical Maximum (7)
        0x35, 0x00,        //   Physical Minimum (0)
        0x46, 0x3B, 0x01,  //   Physical Maximum (315)
        0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
        0x75, 0x04,        //   Report Size (4)
        0x95, 0x01,        //   Report Count (1)
        0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
        //Buttons
        0x65, 0x00,        //   Unit (None)
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x0E,        //   Usage Maximum (0x0E)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x06,        //   Report Count (6)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        //Padding
        0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
        0x09, 0x20,        //   Usage (0x20)
        0x75, 0x06,        //   Report Size (6)
        0x95, 0x01,        //   Report Count (1)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x7F,        //   Logical Maximum (127)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x81, 0x02,
        0xc0
};

static uint8_t send_report[] = {0xa1, 0x11, 0xc0, 0x00, 0x08, 0, 0};

static uint8_t hid_service_buffer[400];
static uint8_t device_id_sdp_service_buffer[400];
static const char hid_device_name[] = DEVICE_NAME;
static uint16_t hid_cid = 0;

static uint8_t but1_send = 0;
static uint8_t but2_send = 0;

static bool connected = false;

static btstack_packet_callback_registration_t hci_event_callback_registration;

void interpretMessage(int length, char *message);

void initBluetooth();

void updateButtons(const char *message);

static void list_link_keys(void);

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size);

static void read_uart() {
    while (1) {
        checkHC05();
        vTaskDelay(2);
    }
}

void initHC05() {
    initBluetooth();
    initUART();
    xTaskCreate(read_uart, "read_uart", 2048, NULL, 1, NULL);
}

void initBluetooth() {
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    hci_register_sco_packet_handler(&packet_handler);
    gap_discoverable_control(1);
    gap_set_class_of_device(0x2508);
    gap_set_local_name(DEVICE_NAME);

    l2cap_init();
    sdp_init();
    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    hid_create_sdp_record(hid_service_buffer, 0x10001, 0x2508, 33, 0, 0, 0, hid_descriptor_gamecube,
                          sizeof(hid_descriptor_gamecube), hid_device_name);
    sdp_register_service(hid_service_buffer);
    device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10003, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH,
                                BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);
    hid_device_init(1, sizeof(hid_descriptor_gamecube), hid_descriptor_gamecube);
    hid_device_register_packet_handler(&packet_handler);

    hci_power_control(HCI_POWER_ON);
}

void checkHC05() {
    char message[50];
    int length = uartRead(message);

    if (length > 0) {
        interpretMessage(length, message);
    }
}

void interpretMessage(int length, char *message) {
    if (inCommandMode) {
        if (strncmp((const char *) message, AUTOCONNECT_REQUEST, length - 1) == 0) { // length-1 cause ending 0x0D
            inCommandMode = false;
            list_link_keys();
        }
    } else {
        if (strncmp((const char *) message, COM_MODE_REQUEST, length) == 0) {
            inCommandMode = true;
            uartWrite(COM_MODE_RESPONSE);
        } else if (connected) {
            // Check headers
            if (message[0] == 0xFD && message[1] == 0x06) {
                // Get HC-05 buttons
                updateButtons(message);
            }
        }
    }
}

void updateButtons(const char *message) {
    uint8_t xAxis = message[2];
    uint8_t yAxis = message[3];
    uint8_t buttons1 = message[6];
    uint8_t buttons2 = message[7];

    int dpad = 0;
    but1_send = 0;
    but2_send = 0;

    if (yAxis == PAD_DOWN) {
        dpad = DPAD_DOWN;
    } else if (yAxis == PAD_UP) {
        dpad = DPAD_UP;
    } else if (xAxis == PAD_LEFT) {
        dpad = DPAD_LEFT;
    } else if (xAxis == PAD_RIGHT) {
        dpad = DPAD_RIGHT;
    } else {
        dpad = DPAD_RELEASED;
    }

    if (buttons1 & BUTTON_A) {
        but1_send += HID_KEY_A;
    }

    if (buttons1 & BUTTON_B) {
        but1_send += HID_KEY_B;
    }

    if (buttons1 & BUTTON_L) {
        but2_send += HID_KEY_L;
    }

    if (buttons1 & BUTTON_R) {
        but2_send += HID_KEY_R;
    }

    if (buttons2 & BUTTON_START) {
        but2_send += HID_KEY_START;
    }

    if (buttons2 & BUTTON_SELECT) {
        but2_send += HID_KEY_SELECT;
    }

    but1_send += dpad;
}


static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size) {
    UNUSED(channel);
    UNUSED(packet_size);
    switch (packet_type) {
        case HCI_EVENT_PACKET:
            switch (packet[0]) {
                case HCI_EVENT_HID_META:
                    switch (hci_event_hid_meta_get_subevent_code(packet)) {
                        case HID_SUBEVENT_CONNECTION_OPENED:
                            if (hid_subevent_connection_opened_get_status(packet)) return;
                            hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                            hid_device_request_can_send_now_event(hid_cid); //start loop
                            log_info("HID Connected");

                            connected = true;

                            uartWrite(CONNECTED_RESULT);

                            break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:
                            log_info("HID Disconnected");
                            hid_cid = 0;
                            break;
                        case HID_SUBEVENT_CAN_SEND_NOW:
                            send_report[4] = but1_send;
                            send_report[5] = but2_send;
                            hid_device_send_interrupt_message(hid_cid, &send_report[0], sizeof(send_report));
                            hid_device_request_can_send_now_event(hid_cid);
                            break;
                        default:
                            break;
                    }
                    break;

                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    printf("Disconnected\n");
                    connected = false;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void list_link_keys(void) {
    bd_addr_t addr;
    link_key_t link_key;
    link_key_type_t type;
    btstack_link_key_iterator_t it;

    int ok = gap_link_key_iterator_init(&it);
    if (!ok) {
        printf("Link key iterator not implemented\n");
        return;
    }
    printf("Stored link keys: \n");
    while (gap_link_key_iterator_get_next(&it, addr, link_key, &type)) {
        printf("%s - type %u, key: ", bd_addr_to_str(addr), (int) type);
        printf_hexdump(link_key, 16);
    }
    printf(".\n");
    gap_link_key_iterator_done(&it);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);


//    if (gap_link_key_iterator_get_next(&it, addr, link_key, &type)) {
//        hid_device_connect(addr, &hid_cid);
//    }
}

esp_err_t save_addr(char *address) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(my_handle, "mac_addr", address, 6);
    if (err != ESP_OK) return err;

    err = nvs_commit(my_handle);
    if (err != ESP_OK) return err;

    nvs_close(my_handle);

    return ESP_OK;
}

void read_addr(char *address) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    nvs_get_blob(my_handle, "mac_addr", address, 6);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    nvs_close(my_handle);
    return ESP_OK;
}