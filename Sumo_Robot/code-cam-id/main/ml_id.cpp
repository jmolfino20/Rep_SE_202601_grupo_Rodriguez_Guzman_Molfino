#include "ml_id.h"
#include "config.h"
#include "model_data.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "esp_log.h"

static const char *TAG = "ML_ID";

static constexpr int kArenaSize = 130 * 1024;
static uint8_t tensor_arena[kArenaSize] __attribute__((aligned(16)));

static tflite::MicroMutableOpResolver<10> s_resolver;

static tflite::MicroInterpreter *s_interpreter = nullptr;
static TfLiteTensor             *s_input        = nullptr;
static TfLiteTensor             *s_output       = nullptr;
static float    s_in_scale = 1.0f;
static int32_t  s_in_zp    = 0;

static const char *CLS[2] = { "Sin_ID", "Con_ID" };

extern "C" void ml_id_init(void) {
    tflite::InitializeTarget();

    /* Ops: mismas que test_curriculum */
    s_resolver.AddConv2D();
    s_resolver.AddDepthwiseConv2D();
    s_resolver.AddRelu();
    s_resolver.AddMaxPool2D();
    s_resolver.AddMean();
    s_resolver.AddFullyConnected();
    s_resolver.AddSoftmax();
    s_resolver.AddQuantize();
    s_resolver.AddDequantize();
    s_resolver.AddReshape();

    const tflite::Model *model = tflite::GetModel(id_detector_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "version mismatch: modelo=%d runtime=%d",
                 (int)model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::MicroInterpreter interpreter(model, s_resolver,
                                                 tensor_arena, kArenaSize);
    s_interpreter = &interpreter;

    if (s_interpreter->AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "AllocateTensors fallo -- arena demasiado pequena (%d bytes)", kArenaSize);
        s_interpreter = nullptr;
        return;
    }

    s_input  = s_interpreter->input(0);
    s_output = s_interpreter->output(0);
    s_in_scale = s_input->params.scale;
    s_in_zp    = s_input->params.zero_point;

    int n_classes = s_output->dims->data[s_output->dims->size - 1];

    ESP_LOGI(TAG, "modelo cargado OK");
    ESP_LOGI(TAG, "  arena usada: %u / %d bytes",
             (unsigned)s_interpreter->arena_used_bytes(), kArenaSize);
    ESP_LOGI(TAG, "  input:  tipo=%d  zp=%d  scale=%.6f  shape=[%d,%d,%d,%d]",
             s_input->type, s_in_zp, s_in_scale,
             s_input->dims->data[0], s_input->dims->data[1],
             s_input->dims->data[2], s_input->dims->data[3]);
    ESP_LOGI(TAG, "  output: tipo=%d  zp=%d  scale=%.6f  n_clases=%d",
             s_output->type, s_output->params.zero_point, s_output->params.scale,
             n_classes);
}

/*
 * Preprocesa QVGA (320x240) → center-crop 240x240 → resize NN 128x128 → int8.
 * Replica exactamente el preprocesamiento de test_curriculum.
 */
static void preprocess_qvga(const uint8_t *src, int8_t *dst) {
    const int SW = 320, CROP = 240, OUT = MODEL_INPUT;
    const int x0 = (SW - CROP) / 2;

    for (int oy = 0; oy < OUT; oy++) {
        int sy = (oy * CROP) / OUT;
        for (int ox = 0; ox < OUT; ox++) {
            int sx = x0 + (ox * CROP) / OUT;
            float pix = src[sy * SW + sx] / 255.0f;
            dst[oy * OUT + ox] = (int8_t)((pix / s_in_scale) + s_in_zp);
        }
    }
}

extern "C" int ml_id_detect(uint8_t *img, int w, int h) {
    if (!s_interpreter) return -1;

    if (w != CAM_WIDTH || h != CAM_HEIGHT) {
        ESP_LOGW(TAG, "tamano incorrecto: %dx%d (esperado %dx%d)", w, h, CAM_WIDTH, CAM_HEIGHT);
        return -1;
    }

    preprocess_qvga(img, s_input->data.int8);

    if (s_interpreter->Invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke fallo");
        return -1;
    }

    /* Binario: output[0]=Sin_ID, output[1]=Con_ID. Devuelve 0 o 1. */
    int8_t *logits = s_output->data.int8;
    int detected = (logits[1] > logits[0]) ? 1 : 0;

    static int prev = -1;
    if (detected != prev) {
        ESP_LOGI(TAG, "-> %s  [sin=%d, con=%d]",
                 CLS[detected], logits[0], logits[1]);
        prev = detected;
    }

    return detected;
}