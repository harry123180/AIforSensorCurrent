#include <Pico_inferencing.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <Adafruit_NeoPixel.h>
// 針腳定義
#define SD_CS 28          // SD卡的片選引腳
#define BUTTON_PIN 27     // 按鈕連接的引腳
#define ADC_PIN 26        // ADC連接的引腳
#define NUMPIXELS 1       // 1個NeoPixel
#define POWER_PIN 11      // 電源控制引腳
#define PIXEL_PIN 12      // NeoPixel引腳
#define SAMPLE_COUNT 256  // 採樣數量
float  samples[SAMPLE_COUNT];     // 儲存採樣數據
int current_predicted_case = -1;  // 用來存儲辨識出來的case編號
int sampleIndex = 0;           // 採樣索引
bool isSampling = false; 
bool done = false;
// 定時器相關
RPI_PICO_Timer ITimer(0);
unsigned long lastTapTime = 0;          // 上次單擊的時間
const unsigned long settingTime = 500;  // 單擊判定的等待時間
bool pendingSingleTap = false;          // 標記是否有待確認的單擊事件
unsigned long samplingStartTime = 0;    // 採樣開始時間
unsigned long samplingEndTime = 0;      // 採樣結束時間
Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static const float features[256] = {
};
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, samples + offset, length * sizeof(float));
    return 0;
}
// 定時器中斷服務程序
bool TimerHandler(repeating_timer *rt) {
  if (isSampling && !done && sampleIndex < SAMPLE_COUNT) {
    samples[sampleIndex] = analogRead(ADC_PIN);  // 讀取ADC值
    sampleIndex++;
  }
  if (sampleIndex >= SAMPLE_COUNT) {
    isSampling = false;
    done = true;
    ITimer.stopTimer();  // 停止採樣
    samplingEndTime = millis();
    Serial.println("數據採集完畢！");
  }
  return true;  // 返回true表示定時器繼續運行
}
//Constants
#define SAMPLING_FREQ_HZ 100
#define SAMPLING_PERIOD_MS 1000 / SAMPLING_FREQ_HZ
#define NUM_CHANNELS EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME
#define NUM_READINGS EI_CLASSIFIER_RAW_SAMPLE_COUNT
#define NUM_CLASSES  EI_CLASSIFIER_LABEL_COUNT
void setup()
{ 
  
    // 初始化NeoPixel
    pixels.begin();
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);  // 打開電源
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    // 設置定時器，每10毫秒觸發一次中斷，用於採樣
    if (ITimer.attachInterruptInterval(1005, TimerHandler)) {  // 10 ms間隔
      Serial.println("定時器設置成功");
    } else {
      Serial.println("定時器設置失敗");
    }
    ITimer.stopTimer(); // 啟動時不立即運行定時器，等待按鈕觸發
}

/**
 * @brief      Arduino main function
 */
void loop()
{   
    // 如果尚未開始採集數據，啟動數據採集
    if (!isSampling && !done) {
        isSampling = true;
        sampleIndex = 0;
        ITimer.restartTimer();  // 啟動定時器開始採樣
        samplingStartTime = millis();
        Serial.println("開始採集數據...");
    }
    //Serial.println(sampleIndex);
    // 如果數據採集完成且可以進行推論
    if (!isSampling && sampleIndex >= SAMPLE_COUNT && done) {
        Serial.println("開始推論...");

        if (sizeof(samples) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            ei_printf("The size of your 'samples' array is not correct. Expected %lu items, but had %lu\n",
                      EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(samples) / sizeof(float));
            delay(1000);
            return;
        }

        ei_impulse_result_t result = { 0 };
        signal_t features_signal;
        features_signal.total_length = SAMPLE_COUNT;
        features_signal.get_data = &raw_feature_get_data;

        // 進行推論
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true /* debug */);
        if (res != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", res);
            return;
        }

        // 顯示推論結果
        print_inference_result(result);

        // 推論完成後的處理
        Serial.println("推論完成！");
        if (current_predicted_case == 0) {
            pixels.clear();
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // 紅色
            pixels.show();
            // 執行 case0 的動作
        } else if (current_predicted_case == 1) {
            pixels.clear();
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));  // 綠色
            pixels.show();
            // 執行 case1 的動作
        } else if (current_predicted_case == 2) {
            pixels.clear();
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));  // 藍色
            pixels.show();
            // 執行 case2 的動作
        }
        done = false;
        delay(1000);  // 暫停 10 秒再重新進行數據採集
        isSampling = false;  // 重置標誌以重新啟動數據採集
    }
  
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);
    // Find the case with the highest prediction score
    float max_score = 0.0;
    current_predicted_case = -1;  // 重置變數

    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        float score = result.classification[i].value;
        ei_printf("  %s: %.5f\r\n", ei_classifier_inferencing_categories[i], score);
        
        // 更新當前回合最高分數及對應的 case
        if (score > max_score) {
            max_score = score;
            current_predicted_case = i;
        }
    }
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