/*
 * test_kd — Inferencia con modelo Student KD (sin KD)
 * Hardware: AI-THINKER ESP32-CAM
 *
 * Flash GPIO 4:
 *   0 Ausente   → apagado
 *   1 Izquierda → PWM ~5%
 *   2 Centro    → PWM ~50%
 *   3 Derecha   → PWM 100%
 *
 * Copiar curriculum_int8.tflite en este directorio antes de compilar.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_camera.h"
#include "driver/ledc.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

static const char *TAG = "simple";

/* ── Modelo embebido (EMBED_FILES en CMakeLists) ─────────────────────────── */
extern const uint8_t model_start[] asm("_binary_student_int8_tflite_start");
extern const uint8_t model_end[]   asm("_binary_student_int8_tflite_end");

/* ── Flash LED ───────────────────────────────────────────────────────────── */
#define FLASH_GPIO    4              /* GPIO_NUM_4 en AI-THINKER               */
#define FLASH_TIMER   LEDC_TIMER_1  /* timer 0 lo usa la cámara para XCLK     */
#define FLASH_CH      LEDC_CHANNEL_1
#define FLASH_FREQ    5000
#define FLASH_RES     LEDC_TIMER_8_BIT   /* duty 0-255 */

static const uint32_t DUTY[4] = { 0, 13, 128, 255 };
static const char    *CLS[4]  = { "Ausente", "Izquierda", "Centro", "Derecha" };

/* ── Pines AI-THINKER ESP32-CAM ──────────────────────────────────────────── */
#define CAM_PWDN   32
#define CAM_RESET  -1
#define CAM_XCLK    0
#define CAM_SIOD   26
#define CAM_SIOC   27
#define CAM_D7     35
#define CAM_D6     34
#define CAM_D5     39
#define CAM_D4     36
#define CAM_D3     21
#define CAM_D2     19
#define CAM_D1     18
#define CAM_D0      5
#define CAM_VSYNC  25
#define CAM_HREF   23
#define CAM_PCLK   22

#define ARENA_KB 500
static uint8_t *tensor_arena = nullptr;

/* ── Cámara ──────────────────────────────────────────────────────────────── */
static esp_err_t camera_init(void)
{
    camera_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.pin_pwdn       = CAM_PWDN;
    cfg.pin_reset      = CAM_RESET;
    cfg.pin_xclk       = CAM_XCLK;
    cfg.pin_sccb_sda   = CAM_SIOD;
    cfg.pin_sccb_scl   = CAM_SIOC;
    cfg.pin_d7 = CAM_D7; cfg.pin_d6 = CAM_D6;
    cfg.pin_d5 = CAM_D5; cfg.pin_d4 = CAM_D4;
    cfg.pin_d3 = CAM_D3; cfg.pin_d2 = CAM_D2;
    cfg.pin_d1 = CAM_D1; cfg.pin_d0 = CAM_D0;
    cfg.pin_vsync      = CAM_VSYNC;
    cfg.pin_href       = CAM_HREF;
    cfg.pin_pclk       = CAM_PCLK;
    cfg.xclk_freq_hz   = 20000000;
    cfg.ledc_timer     = LEDC_TIMER_0;
    cfg.ledc_channel   = LEDC_CHANNEL_0;
    cfg.pixel_format   = PIXFORMAT_GRAYSCALE;
    cfg.frame_size     = FRAMESIZE_QVGA;   /* 320×240; se recorta+reduce a 128×128 */
    cfg.jpeg_quality   = 12;
    cfg.fb_count       = 1;
    cfg.fb_location    = CAMERA_FB_IN_DRAM;
    cfg.grab_mode      = CAMERA_GRAB_WHEN_EMPTY;
    cfg.sccb_i2c_port  = -1;              /* usar puerto I2C interno del driver */
    return esp_camera_init(&cfg);
}

/* ── Flash por PWM ───────────────────────────────────────────────────────── */
static void flash_init(void)
{
    ledc_timer_config_t t;
    memset(&t, 0, sizeof(t));
    t.speed_mode      = LEDC_LOW_SPEED_MODE;
    t.duty_resolution = FLASH_RES;
    t.timer_num       = FLASH_TIMER;
    t.freq_hz         = FLASH_FREQ;
    t.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&t);

    ledc_channel_config_t ch;
    memset(&ch, 0, sizeof(ch));
    ch.gpio_num   = FLASH_GPIO;
    ch.speed_mode = LEDC_LOW_SPEED_MODE;
    ch.channel    = FLASH_CH;
    ch.timer_sel  = FLASH_TIMER;
    ch.duty       = 0;
    ledc_channel_config(&ch);
}

static inline void flash_set(uint8_t cls)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, FLASH_CH, DUTY[cls < 4 ? cls : 0]);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, FLASH_CH);
}

/*
 * Centro-crop QVGA (320×240) → 240×240, resize NN → 128×128, cuantizar.
 * Esto replica el preprocesamiento de prepare_dataset.py.
 */
static void preprocess(const uint8_t *src, int8_t *dst, float scale, int32_t zp)
{
    const int SW = 320, CROP = 240, OUT = 128;
    const int x0 = (SW - CROP) / 2;   /* 40 px desde la izquierda */

    for (int oy = 0; oy < OUT; oy++) {
        int sy = (oy * CROP) / OUT;
        for (int ox = 0; ox < OUT; ox++) {
            int sx = x0 + (ox * CROP) / OUT;
            float pix = src[sy * SW + sx] / 255.0f;
            dst[oy * OUT + ox] = (int8_t)((pix / scale) + zp);
        }
    }
}

/* ── app_main ────────────────────────────────────────────────────────────── */
extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando test_kd...");

    if (camera_init() != ESP_OK) {
        ESP_LOGE(TAG, "Error inicializando cámara"); return;
    }
    ESP_LOGI(TAG, "Cámara lista (QVGA grayscale)");

    flash_init();
    ESP_LOGI(TAG, "Flash LEDC listo (GPIO %d)", FLASH_GPIO);

    /* Tensor arena — PSRAM primero, luego RAM interna */
    tensor_arena = (uint8_t *)heap_caps_malloc(ARENA_KB * 1024, MALLOC_CAP_SPIRAM);
    if (!tensor_arena) {
        ESP_LOGW(TAG, "PSRAM no disponible, usando RAM interna");
        tensor_arena = (uint8_t *)malloc(ARENA_KB * 1024);
    }
    if (!tensor_arena) {
        ESP_LOGE(TAG, "Sin memoria (%d KB)", ARENA_KB); return;
    }

    tflite::InitializeTarget();

    const tflite::Model *tfl = tflite::GetModel(model_start);
    if (tfl->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Versión TFLite incompatible"); return;
    }

    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddRelu();
    resolver.AddMaxPool2D();
    resolver.AddMean();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddReshape();

    static tflite::MicroInterpreter interp(tfl, resolver, tensor_arena, ARENA_KB * 1024);
    if (interp.AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors falló"); return;
    }

    TfLiteTensor *in  = interp.input(0);
    TfLiteTensor *out = interp.output(0);
    float   sc = in->params.scale;
    int32_t zp = in->params.zero_point;

    ESP_LOGI(TAG, "Modelo listo — %d B — input %dx%d int8",
             (int)(model_end - model_start), in->dims->data[1], in->dims->data[2]);

    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) { vTaskDelay(pdMS_TO_TICKS(100)); continue; }

        preprocess(fb->buf, in->data.int8, sc, zp);
        esp_camera_fb_return(fb);

        if (interp.Invoke() != kTfLiteOk) {
            ESP_LOGE(TAG, "Invoke falló"); continue;
        }

        int8_t *logits = out->data.int8;
        uint8_t pred = 0;
        for (int i = 1; i < 4; i++)
            if (logits[i] > logits[pred]) pred = i;

        ESP_LOGI(TAG, "→ %s  [%d,%d,%d,%d]",
                 CLS[pred], logits[0], logits[1], logits[2], logits[3]);
        flash_set(pred);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
