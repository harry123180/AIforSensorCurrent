#include <Button2.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SD.h>
// 針腳定義
#define SD_MOSI 3
#define SD_MISO 4
#define SD_SCK 2
#define SD_CS 28
File myFile;
int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1
Button2 button(27);
int maxSample=0;
char fileName[32] = "";
uint8_t state = 0;  // 使用 uint8_t 存儲 0 到 3 的值，節省內存
uint16_t sampleCounter = 0;  // 使用 uint16_t 存儲 0 到 65535 的值，正好匹配範圍
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
unsigned long lastTapTime = 0; // 記錄上次點擊的時間
const unsigned long settingTime = 500; // 設置的等待時間，例如500毫秒
bool pendingSingleTap = false; // 標記是否有待確認的單擊事件

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pinMode(Power,OUTPUT);
  digitalWrite(Power, HIGH);
  // 初始化 SD 卡
  if (!SD.begin(SD_CS)) {
    Serial.println("初始化失敗!");
    return;
  }
  Serial.println("初始化完成.");
  myFile = SD.open("/"); // 開啟根目錄
  printDirectory(myFile, 0); // 打印所有CSV檔案名
  sampleCounter = maxSample;
  Serial.println(sampleCounter);
  button.setTapHandler([](Button2& b) {
    lastTapTime = millis(); // 更新最後點擊時間
    pendingSingleTap = true; // 標記存在待確認的單擊事件
  });
  button.setDoubleClickHandler([](Button2& b) {
    Serial.println("雙擊");
    pendingSingleTap = false; // 雙擊發生時清除單擊標記
    
    
    
  });
  button.setLongClickHandler([](Button2& b) {
    Serial.println("長按");
    
    pendingSingleTap = false; // 長按發生時清除單擊標記
    state= state+1;
    if(state>=3)
    {
      state=0;
    }
  });
  
}

void loop() {
  button.loop(); // 在循環中調用 button.loop() 以處理按鈕事件。
  checkPendingSingleTap(); // 檢查是否有待確認的單擊事件
  switch (state){
    case 0 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(25, 0, 0));
      pixels.show();
      break;
    case 1 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 25, 0));
      pixels.show();
      break;
    case 2 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 0, 25));
      pixels.show();
      break;
    default:
      Serial.println("D");
      break;
  }

}
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) { // 檢查是否還有文件
      break;
    }
    if (entry.isDirectory()) {
      printDirectory(entry, numTabs + 1); // 遞迴打印該目錄內容，增加縮進
    } else {
      // 檢查檔案名是否以.csv結尾
      String fileName = entry.name();
      if (fileName.endsWith(".csv")) {
        String part1, part2;
        splitFilename(fileName, part1, part2);  // 調用函數
        int num = extractTailNumber(part2);
        if(num>maxSample){
          maxSample=num;
        }

      }
    }
    entry.close();
  }
  
}
void splitFilename(const String& filename, String& part1, String& part2) {
  int firstDotIndex = filename.indexOf('.');  // 查找第一個點號的位置
  if (firstDotIndex != -1) {
    part1 = filename.substring(0, firstDotIndex);  // 獲取第一個點號前的子字符串
    
    int secondDotIndex = filename.indexOf('.', firstDotIndex + 1);  // 查找第二個點號的位置
    if (secondDotIndex != -1) {
      part2 = filename.substring(firstDotIndex + 1, secondDotIndex);  // 獲取第一個和第二個點號之間的子字符串
    }
  }
}
int extractTailNumber(const String& input) {
    int length = input.length(); // 獲取輸入字符串的長度
    int start = -1; // 數字開始的位置初始化為-1

    // 從字符串末尾向前遍歷，尋找第一個非數字字符
    for (int i = length - 1; i >= 0; i--) {
        if (!isDigit(input.charAt(i))) {
            start = i + 1; // 設置數字開始的位置
            break;
        }
    }

    if (start == -1 || start == length) { // 如果沒有找到數字或全是數字
        return 0; // 返回0或根據需要處理異常情況
    }

    // 提取字符串末尾的數字部分並轉換為整數
    String numberStr = input.substring(start);
    return numberStr.toInt(); // 將數字字符串轉換為整數
}
void saveFile(char fileName_[]){
  Serial.println(fileName_);
  // Open the file for writing
  myFile = SD.open(fileName_, FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to ");
    Serial.println(fileName_);
    myFile.println("timestamp,data");
    // Write the first row (0-255)
    for (int i = 0; i < 256; i++) {
      myFile.print(i);
      myFile.print(",");
      float n = 4.0;
      float m = 2.0;
      switch(state){
        case 0:
          n = 4.0;
          m = 4.0;
          break;
        case 1:
          n = 3.0;
          m = 1.5;
          break;
        case 2:
          n = 2.0;
          m = 7.0;
          break;
        default:
          Serial.println("State Error");
          break;
      }
      float randomFloat = (float)rand() / RAND_MAX * n - m;
      myFile.println(randomFloat, 6);  // 6 decimal places
    }
    // Close the file
    myFile.close();
    Serial.println("Done writing.");
  } else {
    Serial.println("Error opening the file.");
  }
}
void checkPendingSingleTap() {
  if (pendingSingleTap && (millis() - lastTapTime > settingTime)) {
    // 如果超過設置的時間後沒有其他事件，確認這是一個單擊
    Serial.println("單擊");
    pendingSingleTap = false; // 重置單擊標記
    snprintf(fileName, sizeof(fileName), "Class%d.data%05d.csv", state, sampleCounter);
    sampleCounter++;
    Serial.print("檔案已儲存   :");
    Serial.println(fileName);
    saveFile(fileName);
  }
}