#include "ble.h"
#include <stdio.h>
#include <string.h>

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "motor.h"

#define DEVICE_NAME "ESP32-VERSTAPPEN"

// UUIDs
#define SERVICE_UUID        0x00FF
#define CHARACTERISTIC_UUID 0xFF01

static uint8_t ble_addr_type;

// 👇 Callback limpio y separado
static int write_cb(uint16_t conn_handle,
                    uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt,
                    void *arg)
{
    if (ctxt->om->om_len < 1) return 0;

    uint8_t cmd = ctxt->om->om_data[0];

    switch (cmd)
    {
        case 0: motor_stop(); break;
        case 3: motor_forward(255, 255); break;
        case 7: motor_backward(255, 255); break;
        case 1: motor_forward(0, 255); break;
        case 5: motor_forward(255, 0); break;
        case 2: motor_forward(120, 255); break;
        case 4: motor_forward(255, 120); break;
        case 6: motor_backward(255, 120); break;
        case 8: motor_backward(120, 255); break;
    }

    return 0;
}

// GATT
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(CHARACTERISTIC_UUID),
                .access_cb = write_cb,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {0}
        }
    },
    {0}
};

static void start_advertising(void)
{
    struct ble_gap_adv_params adv_params = {0};

    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER,
                      &adv_params, NULL, NULL);
}

static void on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    start_advertising();
}

static void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void ble_init(void)
{
    nimble_port_init();

    ble_hs_cfg.sync_cb = on_sync;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_svc_gap_device_name_set(DEVICE_NAME);

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    nimble_port_freertos_init(host_task);
}