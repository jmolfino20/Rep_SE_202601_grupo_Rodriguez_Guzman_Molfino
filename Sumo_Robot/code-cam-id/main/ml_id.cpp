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

    int n_classes = s_output->dims->data[s_output->dims->size - 1];

    ESP_LOGI(TAG, "modelo cargado OK");
    ESP_LOGI(TAG, "  arena usada: %u / %d bytes",
             (unsigned)s_interpreter->arena_used_bytes(), kArenaSize);
    ESP_LOGI(TAG, "  input:  tipo=%d  zp=%d  scale=%.6f  shape=[%d,%d,%d,%d]",
             s_input->type, s_input->params.zero_point, s_input->params.scale,
             s_input->dims->data[0], s_input->dims->data[1],
             s_input->dims->data[2], s_input->dims->data[3]);
    ESP_LOGI(TAG, "  output: tipo=%d  zp=%d  scale=%.6f  n_clases=%d",
             s_output->type, s_output->params.zero_point, s_output->params.scale,
             n_classes);
    ESP_LOGI(TAG, "  idx positivo usado: %d  (umbral=%.2f)",
             (n_classes >= 2) ? 1 : 0, (float)ID_THRESHOLD);
}

extern "C" bool ml_id_detect(uint8_t *img, int w, int h) {
    if (!s_interpreter) return false;

    if (w != IMG_WIDTH || h != IMG_HEIGHT) {
        ESP_LOGW(TAG, "tamano incorrecto: %dx%d (esperado %dx%d)", w, h, IMG_WIDTH, IMG_HEIGHT);
        return false;
    }

    /* Convertir uint8 [0-255] al rango INT8 usando el zero_point real del tensor.
     * Para modelos INT8 estándar: zero_point=-128, lo que resulta en img[i]-128.
     * Hardcodear 128 falla si el modelo fue cuantizado con otro zero_point. */
    int8_t *input_data = s_input->data.int8;
    int32_t in_zp = s_input->params.zero_point;
    for (int i = 0; i < IMG_WIDTH * IMG_HEIGHT; i++) {
        int32_t q = (int32_t)img[i] + in_zp;
        input_data[i] = (int8_t)(q < -128 ? -128 : (q > 127 ? 127 : q));
    }

    if (s_interpreter->Invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Invoke fallo");
        return false;
    }

    /* Para clasificador de 2 clases con softmax: output = [prob_negativa, prob_positiva].
     * Leer índice 0 siempre daba el score de la clase NEGATIVA → nunca superaba 0.5
     * cuando el ID estaba presente. Se elige índice 1 para modelos de 2+ clases. */
    int n_classes = s_output->dims->data[s_output->dims->size - 1];
    int out_idx   = (n_classes >= 2) ? 1 : 0;

    float score;
    if (s_output->type == kTfLiteInt8) {
        int8_t raw = s_output->data.int8[out_idx];
        score = (raw - s_output->params.zero_point) * s_output->params.scale;
    } else if (s_output->type == kTfLiteFloat32) {
        score = s_output->data.f[out_idx];
    } else {
        ESP_LOGE(TAG, "tipo de salida no soportado: %d", s_output->type);
        return false;
    }

    /* Primera inferencia: vuelca todos los scores para verificar el orden de clases */
    static bool first_call = true;
    if (first_call) {
        first_call = false;
        ESP_LOGI(TAG, "PRIMERA INFERENCIA: n_clases=%d  idx_positivo=%d", n_classes, out_idx);
        for (int i = 0; i < n_classes; i++) {
            float s_i;
            if (s_output->type == kTfLiteInt8)
                s_i = (s_output->data.int8[i] - s_output->params.zero_point) * s_output->params.scale;
            else
                s_i = s_output->data.f[i];
            ESP_LOGI(TAG, "  output[%d] = %.4f", i, s_i);
        }
    }

    bool detected = (score >= ID_THRESHOLD);

    /* Loguear solo en cambios de estado para no saturar el monitor */
    static bool prev = false;
    if (detected != prev) {
        if (detected)
            ESP_LOGW(TAG, "ID DETECTADO  (score=%.4f >= %.2f, idx=%d/%d)", score, (float)ID_THRESHOLD, out_idx, n_classes);
        else
            ESP_LOGI(TAG, "ID perdido    (score=%.4f <  %.2f, idx=%d/%d)", score, (float)ID_THRESHOLD, out_idx, n_classes);
        prev = detected;
    }

    return detected;
}
