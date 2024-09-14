#include <Button2.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SD.h>
// Pin definitions
#define SD_MOSI 3
#define SD_MISO 4
#define SD_SCK 2
#define SD_CS 28
File myFile;
int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1
Button2 button(27);
int maxFile=32767;
char fileName[] = "file00001.csv";
int state =0;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pinMode(Power,OUTPUT);
  digitalWrite(Power, HIGH);
  // Initialize SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("Initialization failed!");
    return;
  }
  Serial.println("Initialization done.");
  button.setTapHandler([](Button2& b) {
    Serial.println("Tap");

  });

  button.setDoubleClickHandler([](Button2& b) {
    Serial.println("Double Tap");
    saveFile();
    flash();
    
  });

  button.setLongClickHandler([](Button2& b) {
    Serial.println("Long Click");
    state= state+1;
    if(state>=3)
    {
      state=0;
    }
  });
}

void loop() {
  button.loop(); // 在循環中調用 button.loop() 以處理按鈕事件。
  switch (state){
    case 0 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      break;
    case 1 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.show();
      break;
    case 2 :
      pixels.clear();
      pixels.setPixelColor(0, pixels.Color(0, 0, 255));
      pixels.show();
      break;
    default:
      Serial.println("D");
      break;
  }

}
void saveFile(){
  // Find the next available file name
  for (int i = 1; i <= maxFile; i++) {
    snprintf(fileName, sizeof(fileName), "file%05d.csv", i);
    if (!SD.exists(fileName)) {
      break;
    }
  }
  Serial.println(fileName);
  // Open the file for writing
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to ");
    Serial.println(fileName);

    // Write the first row (0-255)
    for (int i = 0; i < 256; i++) {
      myFile.print(i);
      if (i < 255) {
        myFile.print(",");
      } else {
        myFile.println();
      }
    }

    // Write the second row (random floats between -1.0 and 1.0)
    for (int i = 0; i < 256; i++) {
      float randomFloat = (float)rand() / RAND_MAX * 2.0 - 1.0;
      myFile.print(randomFloat, 6);  // 6 decimal places
      if (i < 255) {
        myFile.print(",");
      } else {
        myFile.println();
      }
    }

    // Close the file
    myFile.close();
    Serial.println("Done writing.");
  } else {
    Serial.println("Error opening the file.");
  }

}
void flash(){
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();
  delay(300);
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  delay(300);
}