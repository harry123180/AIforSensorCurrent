#include <Button2.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SD.h>
#include <RPi_Pico_TimerInterrupt.h>

// 針腳定義
#define SD_CS 28          // SD卡的片選引腳
#define BUTTON_PIN 27     // 按鈕連接的引腳
#define ADC_PIN 26        // ADC連接的引腳
#define NUMPIXELS 1       // 1個NeoPixel
#define POWER_PIN 11      // 電源控制引腳
#define PIXEL_PIN 12      // NeoPixel引腳
#define SAMPLE_COUNT 256  // 採樣數量

// 全域變數
File myFile;
int maxSample = 200;
char fileName[32] = "";
uint8_t state = 0;             // 用於控制不同的採樣分類
uint16_t sampleCounter = 200;    // 用於生成不同的文件編號
int samples[SAMPLE_COUNT];     // 儲存採樣數據
int sampleIndex = 0;           // 採樣索引
bool isSampling = false;       // 標記是否正在採樣

Adafruit_NeoPixel pixels(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Button2 button(BUTTON_PIN);    // 按鈕實例

// 定時器相關
RPI_PICO_Timer ITimer(0);
unsigned long lastTapTime = 0;          // 上次單擊的時間
const unsigned long settingTime = 500;  // 單擊判定的等待時間
bool pendingSingleTap = false;          // 標記是否有待確認的單擊事件
unsigned long samplingStartTime = 0;    // 採樣開始時間
unsigned long samplingEndTime = 0;      // 採樣結束時間

// 初始化函數
void setup() {
  Serial.begin(115200);

  // 初始化NeoPixel
  pixels.begin();
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);  // 打開電源

  // 初始化SD卡
  if (!SD.begin(SD_CS)) {
    Serial.println("SD卡初始化失敗!");
    return;
  }
  Serial.println("SD卡初始化完成.");
  myFile = SD.open("/");  // 打開根目錄
  printDirectory(myFile, 0);  // 列出所有CSV文件名
  sampleCounter = maxSample;
  Serial.println(sampleCounter);

  // 設置按鈕事件
  button.setTapHandler([](Button2& b) {
    lastTapTime = millis();  // 更新最後點擊時間
    pendingSingleTap = true;  // 標記存在待確認的單擊事件
  });
  button.setDoubleClickHandler([](Button2& b) {
    Serial.println("雙擊");
    pendingSingleTap = false;  // 雙擊發生時清除單擊標記
  });
  button.setLongClickHandler([](Button2& b) {
    Serial.println("長按");
    pendingSingleTap = false;  // 長按發生時清除單擊標記
    state = (state + 1) % 3;   // 切換狀態
  });

  // 設置定時器，每10毫秒觸發一次中斷，用於採樣
  if (ITimer.attachInterruptInterval(1005, TimerHandler)) {  // 10 ms間隔
    Serial.println("定時器設置成功");
  } else {
    Serial.println("定時器設置失敗");
  }
  ITimer.stopTimer(); // 啟動時不立即運行定時器，等待按鈕觸發
}

// 定時器中斷服務程序
bool TimerHandler(repeating_timer *rt) {
  if (isSampling && sampleIndex < SAMPLE_COUNT) {
    samples[sampleIndex] = analogRead(ADC_PIN);  // 讀取ADC值
    sampleIndex++;
  }
  if (sampleIndex >= SAMPLE_COUNT) {
    isSampling = false;
    ITimer.stopTimer();  // 停止採樣
    samplingEndTime = millis();
    Serial.println("數據採集完畢！");
    saveFile(fileName);  // 保存數據到SD卡
  }
  return true;  // 返回true表示定時器繼續運行
}

// 主循環函數
void loop() {
  button.loop();  // 處理按鈕事件
  checkPendingSingleTap();  // 檢查單擊事件

  // 控制NeoPixel顏色指示不同狀態
  switch (state) {
    case 0:
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(25, 0, 0));  // 紅色
      pixels.show();
      break;
    case 1:
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 25, 0));  // 綠色
      pixels.show();
      break;
    case 2:
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 0, 25));  // 藍色
      pixels.show();
      break;
    default:
      Serial.println("未知狀態");
      break;
  }
}

// 檢查是否有待確認的單擊事件
void checkPendingSingleTap() {
  if (pendingSingleTap && (millis() - lastTapTime > settingTime)) {
    Serial.println("單擊");
    pendingSingleTap = false;  // 重置單擊標記
    snprintf(fileName, sizeof(fileName), "case%d.data%05d.csv", state, sampleCounter);
    sampleCounter++;
    Serial.print("文件保存: ");
    Serial.println(fileName);
    startSampling();
  }
}

// 開始採樣的函數
void startSampling() {
  sampleIndex = 0;
  isSampling = true;
  samplingStartTime = millis();
  ITimer.restartTimer();  // 啟動定時器開始採樣
  Serial.println("開始採集數據...");
}

// 保存數據到文件
void saveFile(char fileName_[]) {
  Serial.println(fileName_);
  myFile = SD.open(fileName_, FILE_WRITE);
  if (myFile) {
    Serial.print("寫入文件: ");
    Serial.println(fileName_);
    myFile.println("timestamp,data");
    for (int i = 0; i < SAMPLE_COUNT; i++) {
      myFile.print(i);
      myFile.print(",");
      myFile.println(samples[i]);
    }
    unsigned long totalTime = samplingEndTime - samplingStartTime;
    Serial.print("數據採集總耗時: ");
    Serial.print(totalTime);
    Serial.println(" 毫秒");
    myFile.close();
    Serial.println("文件寫入完成.");
  } else {
    Serial.println("打開文件失敗.");
  }
}

// 列出目錄中的文件
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;  // 無更多文件
    }
    if (entry.isDirectory()) {
      printDirectory(entry, numTabs + 1);
    } else {
      String fileName = entry.name();
      if (fileName.endsWith(".csv")) {
        String part1, part2;
        splitFilename(fileName, part1, part2);
        int num = extractTailNumber(part2);
        if (num > maxSample) {
          maxSample = num;
        }
      }
    }
    entry.close();
  }
}

// 分割文件名
void splitFilename(const String& filename, String& part1, String& part2) {
  int firstDotIndex = filename.indexOf('.');
  if (firstDotIndex != -1) {
    part1 = filename.substring(0, firstDotIndex);
    int secondDotIndex = filename.indexOf('.', firstDotIndex + 1);
    if (secondDotIndex != -1) {
      part2 = filename.substring(firstDotIndex + 1, secondDotIndex);
    }
  }
}

// 提取文件名中的數字
int extractTailNumber(const String& input) {
  int length = input.length();
  int start = -1;
  for (int i = length - 1; i >= 0; i--) {
    if (!isDigit(input.charAt(i))) {
      start = i + 1;
      break;
    }
  }
  if (start == -1 || start == length) {
    return 0;
  }
  String numberStr = input.substring(start);
  return numberStr.toInt();
}
