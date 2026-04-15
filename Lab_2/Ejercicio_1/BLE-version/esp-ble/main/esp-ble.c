#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define DEVICE_NAME "ESP32-VERSTAPPEN"

// UUIDs (16-bit)
#define SERVICE_UUID        0x00FF
#define CHARACTERISTIC_UUID 0xFF01

static uint8_t ble_addr_type;

static uint8_t last_cmd = 0xFF;

static const char* cmd_to_str(uint8_t cmd)
{
    switch (cmd) {
        case 0: return "STOP";
        case 1: return "IZQUIERDA";
        case 2: return "ARRIBA-IZQUIERDA";
        case 3: return "ARRIBA";
        case 4: return "ARRIBA-DERECHA";
        case 5: return "DERECHA";
        case 6: return "ABAJO-DERECHA";
        case 7: return "ABAJO";
        case 8: return "ABAJO-IZQUIERDA";
        default: return "DESCONOCIDO";
    }
}

static int write_cb(uint16_t conn_handle,
                    uint16_t attr_handle,
                    struct ble_gatt_access_ctxt *ctxt,
                    void *arg)
{
    if (ctxt->om->om_len < 1) return 0;

    uint8_t cmd = ctxt->om->om_data[0];

    if (cmd != last_cmd) {
        printf("%s (%d)\n", cmd_to_str(cmd), cmd);
        last_cmd = cmd;
    }

    return 0;
}

// Definición de servicio y característica
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(SERVICE_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(CHARACTERISTIC_UUID),
                .access_cb = write_cb,
                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP, // tipo "UDP"
            },
            {0}
        }
    },
    {0}
};

// Hace que se pueda conectar a un PC nuevo
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

// Sync (cuando BLE está listo)
static void on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    start_advertising();
}

// Task principal NimBLE
void host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void)
{
    // Manejo correcto de NVS (evita errores raros)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    nimble_port_init();

    ble_hs_cfg.sync_cb = on_sync;

    // GAP + GATT
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_svc_gap_device_name_set(DEVICE_NAME);

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    nimble_port_freertos_init(host_task);
}