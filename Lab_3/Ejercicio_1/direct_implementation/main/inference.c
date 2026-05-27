#include "inference.h"
#include "model.h"

#include "esp_err.h"
#include "dsps_dotprod.h"

static inline float relu(float x)
{
    return (x > 0.0f) ? x : 0.0f;
}

void inference(float x, float *y_out)
{
    float layer1[HIDDEN1_SIZE];
    float layer2[HIDDEN2_SIZE];

    // Layer 1
    for (int i = 0; i < HIDDEN1_SIZE; i++) {

        layer1[i] = relu(x * W1[i] + B1[i]);
    }

    // Layer 2
    for (int i = 0; i < HIDDEN2_SIZE; i++) {

        float dot = 0.0f;

        ESP_ERROR_CHECK(
            dsps_dotprod_f32(
                layer1,
                W2[i],
                &dot,
                HIDDEN1_SIZE
            )
        );

        float sum = dot + B2[i];

        layer2[i] = relu(sum);
    }

    // Output Layer
    float dot = 0.0f;

    ESP_ERROR_CHECK(
        dsps_dotprod_f32(
            layer2,
            W3,
            &dot,
            HIDDEN2_SIZE
        )
    );

    *y_out = dot + B3;
}
