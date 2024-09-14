#include <Button2.h>
#include <Adafruit_NeoPixel.h>
int Power = 11;
int PIN  = 12;
#define NUMPIXELS 1
Button2 button(27);

int state =0;
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pinMode(Power,OUTPUT);
  digitalWrite(Power, HIGH);
  button.setTapHandler([](Button2& b) {
    Serial.println("Tap");
  });

  button.setDoubleClickHandler([](Button2& b) {
    Serial.println("Double Tap");
    
    
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
  Serial.println(state);
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
    