//
// Created by Shyri on 2019-09-14.
//

#include <stdio.h>
#include <esp_log.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_hidd_profile_api.h"

static uint16_t hid_conn_id = 0;
static bool sec_conn = false;
static bool send_volum_up = false;
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

#define HID_DEMO_TAG "HID_DEMO"
#define LOG_TAG "BLUETOOTH"

static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param);

#define HIDD_DEVICE_NAME            "GBA Bluetooth"

static uint8_t hidd_service_uuid128[] = {
        /* LSB <--------------------------------------------------------------------------------> MSB */
        //first uuid, 16bit, [12],[13] is the value
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};

static esp_ble_adv_data_t hidd_adv_data = {
        .set_scan_rsp = false,  // Set this advertising data as scan response or not
        .include_name = true,   // Advertising data include device name or not
        .include_txpower = true,// Advertising data include TX power
        .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
        .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
        .appearance = 0x03C4,   // Gamepad,
        .manufacturer_len = 0,  // Manufacturer data length
        .p_manufacturer_data =  NULL,   // Manufacturer data point
        .service_data_len = 0,  // Service data length
        .p_service_data = NULL, // Service data point
        .service_uuid_len = sizeof(hidd_service_uuid128),   // Service uuid length
        .p_service_uuid = hidd_service_uuid128, // Service uuid array point
        .flag = 0x6,    // Advertising flag of discovery mode
};

static esp_ble_adv_params_t hidd_adv_params = {
        .adv_int_min        = 0x20, // From 0x20 ms
        .adv_int_max        = 0x30, // To 0x30 ms
        .adv_type           = ADV_TYPE_IND, // Generic, not directed to a particular central device and advertises the server as connectable
        .own_addr_type      = BLE_ADDR_TYPE_PUBLIC, // Address type public
        //.peer_addr            =
        //.peer_addr_type       =
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};


static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param) {
    switch (event) {
        case ESP_HIDD_EVENT_REG_FINISH: {
            ESP_LOGE(LOG_TAG, "ESP_HIDD_EVENT_REG_FINISH");
            if (param->init_finish.state == ESP_HIDD_INIT_OK) {
                ESP_LOGE(LOG_TAG, "ESP_HIDD_INIT_OK");
                //esp_bd_addr_t rand_addr = {0x04,0x11,0x11,0x11,0x11,0x05};
                esp_ble_gap_set_device_name(HIDD_DEVICE_NAME);
                esp_ble_gap_config_adv_data(&hidd_adv_data);
            }
            break;
        }
        case ESP_BAT_EVENT_REG: {
            ESP_LOGE(LOG_TAG, "ESP_BAT_EVENT_REG");
            break;
        }
        case ESP_HIDD_EVENT_DEINIT_FINISH:
            break;
        case ESP_HIDD_EVENT_BLE_CONNECT: {
            ESP_LOGI(HID_DEMO_TAG, "ESP_HIDD_EVENT_BLE_CONNECT");
            hid_conn_id = param->connect.conn_id;
            break;
        }
        case ESP_HIDD_EVENT_BLE_DISCONNECT: {
            sec_conn = false;
            ESP_LOGI(HID_DEMO_TAG, "ESP_HIDD_EVENT_BLE_DISCONNECT");
            esp_ble_gap_start_advertising(&hidd_adv_params);
            break;
        }
        case ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT: {
            ESP_LOGI(HID_DEMO_TAG, "%s, ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT", __func__);
            ESP_LOG_BUFFER_HEX(HID_DEMO_TAG, param->vendor_write.data, param->vendor_write.length);
        }
        default:
            break;
    }
    return;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT");
            esp_ble_gap_start_advertising(&hidd_adv_params);
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            for (int i = 0; i < ESP_BD_ADDR_LEN; i++) {
                ESP_LOGD(HID_DEMO_TAG, "%x:", param->ble_security.ble_req.bd_addr[i]);
            }
            ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_SEC_REQ_EVT");
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            ESP_LOGI(LOG_TAG, "ESP_GAP_BLE_AUTH_CMPL_EVT");
            sec_conn = true;
            esp_bd_addr_t bd_addr;
            memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            ESP_LOGI(HID_DEMO_TAG, "remote BD_ADDR: %08x%04x", \
                (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                     (bd_addr[4] << 8) + bd_addr[5]);
            ESP_LOGI(HID_DEMO_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
            ESP_LOGI(HID_DEMO_TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
            if (!param->ble_security.auth_cmpl.success) {
                ESP_LOGE(HID_DEMO_TAG, "fail reason = 0x%x", param->ble_security.auth_cmpl.fail_reason);
            }
            break;
        default:
            break;
    }
}

void hid_demo_task(void *pvParameters) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        if (sec_conn) {
            ESP_LOGI(HID_DEMO_TAG, "Send the volume");
            send_volum_up = true;
            //uint8_t key_vaule = {HID_KEY_A};
            //esp_hidd_send_keyboard_value(hid_conn_id, 0, &key_vaule, 1);
//            esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_UP, true);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            if (send_volum_up) {
                send_volum_up = false;
//                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_UP, false);
//                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_DOWN, true);
                vTaskDelay(3000 / portTICK_PERIOD_MS);
//                esp_hidd_send_consumer_value(hid_conn_id, HID_CONSUMER_VOLUME_DOWN, false);
            }
        }
    }
}

void initBluetooth() {
    esp_err_t ret;

    // Initialize Non-Volatile Memory
    // NVS allows to save key-value pairs in flash memory (useful to store MAC address to reconnect for example)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Initialize BT controller with standard configuration
    // BT Controller implements Host Controller Interface (HCI), Link Layer (LL) and Physical Layer (PL)
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(HID_DEMO_TAG, "%s initialize controller failed\n", __func__);
        return;
    }
    // Enable the BT Controller in BLE Mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(HID_DEMO_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    // Initialize Bluedroid stack, includes common definitions and API for both BT Classic and BLE
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(HID_DEMO_TAG, "%s init bluedroid failed\n", __func__);
        return;
    }

    // Enable bluedroid
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(HID_DEMO_TAG, "%s init bluedroid failed\n", __func__);
        return;
    }

    // Initialize Bluetooth HID Profile
    if ((ret = esp_hidd_profile_init()) != ESP_OK) {
        ESP_LOGE(HID_DEMO_TAG, "%s init bluedroid failed\n", __func__);
    }

    ///register the callback function to the gap module
    // Register GAP event handler
    // Generic Access Profile is what makes your device visible to the outside world, and determines how two devices can (or can't) interact with each other.
    esp_ble_gap_register_callback(gap_event_handler);

    // Register HID event callbacks
    esp_hidd_register_callbacks(hidd_event_callback);

    // set the security iocap & auth_req & key size & init key response key parameters to the stack
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;     // Bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           // Set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));

    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

    ESP_LOGI(LOG_TAG, "Creating Tasks");

    xTaskCreate(&hid_demo_task, "hid_task", 2048, NULL, 5, NULL);
}