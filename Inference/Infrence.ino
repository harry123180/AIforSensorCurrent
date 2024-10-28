
#include <Pico_inferencing.h>

static const float features[256] = {
   -5.3904, -6.6514, -6.6599, -6.6798, -5.9382, 
   -6.3123, -5.8246, -6.9626, -6.3854, -6.8338, 
   -5.5090, -6.0766, -5.6009, -5.7341, -5.2229, 
   -6.4288, -6.0699, -5.2897, -6.0325, -5.9278, 
   -5.1759, -6.7294, -5.8231, -5.1757, -5.3512, 
   -5.2162, -6.7914, -6.7973, -5.9086, -5.3383, 
   -5.4095, -5.3281, -5.5087, -6.9923, -6.0881, 
   -6.6487, -6.6183, -5.9106, -6.8199, -5.0507, 
   -5.3416, -6.2319, -6.8516, -6.5069, -6.5915, 
   -6.4642, -5.1990, -6.9768, -5.3841, -6.4658, 
   -5.3018, -5.9492, -5.0528, -6.3085, -5.0748, 
   -5.6881, -5.1645, -6.9146, -5.4390, -5.7220, 
   -5.9024, -6.8884, -5.6594, -6.2561, -5.5743, 
   -5.9601, -6.5276, -6.8229, -5.9589, -6.4442, 
   -5.8071, -6.2423, -5.0808, -6.5873, -5.2920, 
   -6.0939, -5.7993, -5.0199, -5.3871, -5.7357, 
   -6.1928, -5.7660, -6.2177, -5.7413, -5.4639, 
   -6.1062, -5.2228, -6.8025, -5.7361, -5.2620, 
   -6.9532, -6.7490, -6.7479, -6.3077, -5.2849, 
   -6.5398, -5.8592, -5.7554, -6.1325, -5.5691, 
   -5.6633, -5.4946, -6.6717, -6.3293, -5.6432, 
   -5.8485, -5.4893, -5.1021, -6.6151, -5.6929, 
   -5.1556, -5.1703, -6.0392, -6.4063, -6.0330, 
   -6.4372, -6.3954, -5.4504, -6.5432, -5.6943, 
   -5.1469, -6.0332, -6.7758, -5.5241, -5.9227, 
   -5.8123, -6.1334, -5.0595, -5.6917, -6.2385, 
   -6.7297, -6.7034, -6.8730, -5.9186, -6.8925, 
   -5.1977, -6.9111, -5.4767, -5.3264, -6.5618, 
   -5.8566, -6.5462, -5.4536, -5.6509, -6.1086, 
   -5.9429, -6.5612, -5.1530, -5.5811, -6.0619, 
   -6.5719, -6.8082, -6.4888, -5.6649, -5.1787, 
   -6.5302, -6.9008, -5.1717, -6.8462, -6.1648, 
   -6.0440, -6.7710, -6.7043, -5.7164, -6.8784, 
   -6.0339, -5.6286, -6.3536, -6.7633, -6.3985, 
   -5.9253, -5.8093, -5.1140, -5.9080, -5.7341,
   -6.4544, -5.3109, -5.0289, -5.4886, -6.7542, 
   -6.0828, -5.3932, -5.7381, -6.5446, -6.3109, 
   -5.5777, -5.3997, -5.6118, -6.1949, -5.0801, 
   -6.2414, -6.0583, -6.2882, -6.8569, -6.9143, 
   -5.9636, -5.8128, -6.5596, -5.2876, -6.6054, 
   -5.3349, -6.7010, -6.6091, -6.9397, -6.9038, 
   -5.5433, -5.5793, -5.2762, -6.5476, -6.9247, 
   -5.6661, -6.7333, -5.2886, -5.6436, -6.2071, 
   -6.9726, -5.0751, -6.4053, -5.0646, -5.4465, 
   -5.4378, -5.4676, -5.4932, -5.3938, -5.3775, 
   -6.3067, -5.8892, -5.9054, -5.4990, -5.4807, 
   -5.5922, -6.5732, -6.8174, -5.4960, -5.5014, 
   -5.5631, -5.3031, -6.1407, -6.9313, -5.6187, 
   -5.6815, -5.6781, -5.6584, -5.9282, -6.9131, 
   -6.8345, -5.3821, -6.1637, -6.1248, -6.3133, 
   -5.9225, -6.0588, -5.4081, -5.1728, -5.7540, 
   -6.4390
};
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}
//Constants
#define SAMPLING_FREQ_HZ 100
#define SAMPLING_PERIOD_MS 1000 / SAMPLING_FREQ_HZ
#define NUM_CHANNELS EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME
#define NUM_READINGS EI_CLASSIFIER_RAW_SAMPLE_COUNT
#define NUM_CLASSES  EI_CLASSIFIER_LABEL_COUNT
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
}

/**
 * @brief      Arduino main function
 */
void loop()
{
    ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(1000);
        return;
    }

    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true /* debug */);
    if (res != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", res);
        return;
    }

    // print inference return code
    ei_printf("run_classifier returned: %d\r\n", res);
    print_inference_result(result);

    delay(10000);
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);

    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }

    // Print the prediction results (classification)
#else
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
    ei_printf("Visual anomalies:\r\n");
    for (uint32_t i = 0; i < result.visual_ad_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.visual_ad_grid_cells[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }
#endif

}