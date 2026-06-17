#include "ml_id.h"
#include "config.h"
#include "model_data.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "esp_log.h"

static const char *TAG = "ML_ID";

/*
 * Tensor arena: buffer de trabajo para activaciones intermedias y scratch de kernels.
 * El uso real se loguea en ml_id_init — ajustar kArenaSize si AllocateTensors falla.
 * 100 KB es suficiente para una CNN INT8 con entrada 96x96x1.
 */
static constexpr int kArenaSize = 100 * 1024;
static uint8_t tensor_arena[kArenaSize] __attribute__((aligned(16)));

/*
 * Ops registradas para cubrir arquitecturas CNN de clasificación típicas
 * (Conv2D, Depthwise, Pooling, Dense, Reshape, activaciones, quant/dequant).
 * El parámetro de template debe ser >= cantidad de Add* llamados abajo.
 */
static tflite::MicroMutableOpResolver<10> s_resolver;

static tflite::MicroInterpreter *s_interpreter = nullptr;
static TfLiteTensor             *s_input        = nullptr;
static TfLiteTensor             *s_output       = nullptr;

extern "C" void ml_id_init(void) {
    tflite::InitializeTarget();

    /* Registrar ops antes de crear el intérprete */
    s_resolver.AddConv2D();
    s_resolver.AddDepthwiseConv2D();
    s_resolver.AddMaxPool2D();
    s_resolver.AddAveragePool2D();
    s_resolver.AddFullyConnected();
    s_resolver.AddReshape();
    s_resolver.AddSoftmax();
    s_resolver.AddLogistic();
    s_resolver.AddDequantize();
    s_resolver.AddQuantize();

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

    ESP_LOGI(TAG, "modelo cargado OK");
    ESP_LOGI(TAG, "  arena usada: %u / %d bytes",
             (unsigned)s_interpreter->arena_used_bytes(), kArenaSize);
    ESP_LOGI(TAG, "  input:  tipo=%d  shape=[%d,%d,%d,%d]",
             s_input->type,
             s_input->dims->data[0], s_input->dims->data[1],
             s_input->dims->data[2], s_input->dims->data[3]);
    ESP_LOGI(TAG, "  output: tipo=%d  scale=%.6f  zero_point=%d",
             s_output->type,
             s_output->params.scale, s_output->params.zero_point);
}

extern "C" bool ml_id_detect(uint8_t *img, int w, int h) {
    if (!s_interpreter) return false;

    if (w != IMG_WIDTH || h != IMG_HEIGHT) {
        ESP_LOGW(TAG, "tamano incorrecto: %dx%d (esperado %dx%d)", w, h, IMG_WIDTH, IMG_HEIGHT);
        return false;
    }

    /* Convertir uint8 [0-255] a int8 [-128,127] que espera el tensor INT8 */
    int8_t *input_data = s_input->data.int8;
    for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
        input_data[i] = (int8_t)((int)img[i] - 128);
    }

    if (s_interpreter->Invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke fallo");
        return false;
    }

    /* Dequantizar salida INT8 -> float via scale/zero_point del tensor */
    float score;
    if (s_output->type == kTfLiteInt8) {
        int8_t raw = s_output->data.int8[0];
        score = (raw - s_output->params.zero_point) * s_output->params.scale;
    } else if (s_output->type == kTfLiteFloat32) {
        score = s_output->data.f[0];
    } else {
        ESP_LOGE(TAG, "tipo de salida no soportado: %d", s_output->type);
        return false;
    }

    bool detected = (score >= ID_THRESHOLD);

    /* Loguear solo en cambios de estado para no saturar el monitor */
    static bool prev = false;
    if (detected != prev) {
        if (detected) {
            ESP_LOGW(TAG, "ID DETECTADO  (score=%.4f >= %.2f)", score, (float)ID_THRESHOLD);
        } else {
            ESP_LOGI(TAG, "ID perdido    (score=%.4f <  %.2f)", score, (float)ID_THRESHOLD);
        }
        prev = detected;
    }

    return detected;
}
