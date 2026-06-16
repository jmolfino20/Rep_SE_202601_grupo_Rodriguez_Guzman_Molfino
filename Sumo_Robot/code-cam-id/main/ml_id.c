#include "ml_id.h"
#include "config.h"
#include <stdio.h>

/*
 * TODO – TFLite Micro integration
 *
 * When the model is trained:
 *  1. Export as TFLite flatbuffer:  model.tflite
 *  2. Convert to C array:           xxd -i model.tflite > model_data.h
 *  3. Uncomment esp-tflite-micro in idf_component.yml
 *  4. Replace this stub with TFLite inference code
 *
 * Expected model interface:
 *   Input:  uint8 grayscale [IMG_HEIGHT × IMG_WIDTH]  (0–255)
 *   Output: float[1]  → value ≥ ID_THRESHOLD means "ID present"
 */

void ml_id_init(void) {
    printf("ml_id: stub – no model loaded\n");
}

bool ml_id_detect(uint8_t *img, int w, int h) {
    (void)img; (void)w; (void)h;
    return false;
}
