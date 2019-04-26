#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // using the 128x32 display
#include <qrcode.h>  
#include "heartRate.h"
#include "MAX30105.h"

#define OLED_DC   PB10  //   D/C   DC
#define OLED_RST  PB1   //   RST   RES
#define OLED_MOSI PB0   //   SDA   D1
#define OLED_CLK  PA7   //   SCL   D0
#define OLED_CS   PB11  //   ---   x Not Connected
#define KEY       PB12  //   按键

Adafruit_SSD1306 OLED(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //增加这个，以便更平均。4是好的。
const byte SAMP_SIZE = 127;
const byte HEIGHT = 8;
byte rates[SAMP_SIZE]; //一系列的心率
byte rateSpot = 0;
long lastBeat = 0; //最后一拍发生的时间

float beatsPerMinute;
int beatAvg;
long delta;

int maxx = -1;
int seen = 0;
int key,key_nc;
const char *QR = "HELLO！"; //"http://qm.qq.com/cgi-bin/qm/qr?k=xLg2jipz7Mh3XAdtDlyVhk-I0VFTH_jc";

extern uint8_t str[]{
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x7C,0x00,0x00,0xFF,0x81,0xFF,0x00,0x01,
0xFF,0xC3,0xFF,0xC0,0x03,0xFF,0xE7,0xFF,0xE0,0x07,0xFF,0xFF,0xFF,0xF0,0x0F,0xFF,
0xFF,0xFF,0xF0,0x1F,0xFF,0xFF,0xFF,0xF8,0x1F,0xFF,0xFF,0xFF,0xF8,0x3F,0xFF,0xFF,
0xFF,0xFC,0x3F,0xFF,0xFF,0xFF,0xFC,0x3F,0xFF,0xFF,0xFF,0xFC,0x3F,0xFF,0xFF,0xFF,
0xF8,0x3F,0xFF,0xFF,0xFF,0xF8,0x1F,0xFF,0xFF,0xFF,0xF8,0x0F,0xFF,0xFF,0xFF,0xF0,
0x07,0xFF,0xFF,0xFF,0xE0,0x07,0xFF,0xFF,0xFF,0xC0,0x03,0xFF,0xFF,0xFF,0x80,0x00,
0xFF,0xFF,0xFF,0x00,0x00,0x7F,0xFF,0xFE,0x00,0x00,0x7F,0xFF,0xFC,0x00,0x00,0x3F,
0xFF,0xF8,0x00,0x00,0x1F,0xFF,0xF0,0x00,0x00,0x07,0xFF,0xE0,0x00,0x00,0x01,0xFF,
0xC0,0x00,0x00,0x00,0xFF,0x80,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x3E,0x00,
0x00,0x00,0x00,0x1C,0x00,0x00
};

void setup() {
  OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial1.begin(9600);
  pinMode(KEY, INPUT_PULLUP);
  digitalWrite(KEY, HIGH);
  reset();
  OLED.println("Starting up...");   //启动……
  OLED.display();
  delay(500);

  //初始化传感器
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    OLED.println("No sensor. Please check wiring/power.");  //没有传感器。请检查连接/电源。
    OLED.display();
    while (1);
  }

  reset();
  //OLED.println("Put Finger On Sensor");  //把手指放在感应器上。
  
  OLED.display();

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
}

void loop() {
  long irValue = particleSensor.getIR();
  key = digitalRead(KEY);
  if(key != key_nc && key == LOW){
    QR_cude(QR);
    delay(10000);
    OLED.clearDisplay();
  }
  int x1,x2,x3,x4,x5,x6,x7;
  int y1,y2,y3,y4,y5,y6,y7;
  if (irValue > 50000 && !seen) {
    reset();
    randomSeed(analogRead(0));
    x1 = random(24,48);x2 = random(24,48);x3 = random(24,48);
    x4 = random(24,48);x5 = random(24,48);x6 = random(24,48);
    x7 = random(24,48);
    y1 = random(24,48);y2 = random(24,48);y3 = random(24,48);
    y4 = random(24,48);y5 = random(24,48);y6 = random(24,48);
    y7 = random(24,48);
    OLED.setCursor(12,0);
    OLED.print("Detect Heartbeats");  //收集数据……
    OLED.drawBitmap(44, 20, str, 36, 30, WHITE);
    OLED.drawLine(0,36,20,36, WHITE);
    OLED.drawLine(20,36,23,x1, WHITE);
    OLED.drawLine(23,x1,26,x2, WHITE);
    OLED.drawLine(26,x2,29,x3, WHITE);
    OLED.drawLine(29,x3,32,x4, WHITE);
    OLED.drawLine(32,x4,35,x5, WHITE);
    OLED.drawLine(35,x5,38,x6, WHITE);
    OLED.drawLine(38,x6,41,x7, WHITE);
    OLED.drawLine(41,x7,46,36, WHITE);
    //
    //OLED.drawLine(46,36,80,36, WHITE);
    //
    OLED.drawLine(80,36,85,y1, WHITE);
    OLED.drawLine(85,y1,88,y2, WHITE);
    OLED.drawLine(88,y2,91,y3, WHITE);
    OLED.drawLine(91,y3,94,y4, WHITE);
    OLED.drawLine(94,y4,97,y5, WHITE);
    OLED.drawLine(97,y5,100,y6, WHITE);
    OLED.drawLine(100,y6,103,y7, WHITE);
    OLED.drawLine(103,y7,106,36, WHITE);
    OLED.drawLine(106,36,128,36, WHITE);
    OLED.display();
  }

  if (checkForBeat(irValue)) {
    //我们感觉到有节奏!
    long now = millis();
    delta = now - lastBeat;
    lastBeat = now;

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20 && delta < 1000) {
      int measure = (byte) beatsPerMinute;
      rates[rateSpot++] = measure;
      rateSpot %= SAMP_SIZE; // Cyclic buffer

      // Take average of last 4 readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[(rateSpot - x - 1) % SAMP_SIZE];
      }
      beatAvg /= RATE_SIZE;

      // calc height
      if (measure > maxx) {
        maxx = measure;
      }
      int tmp = maxx / HEIGHT;

      reset();
      seen = 1;
      OLED.setTextColor(WHITE);        //设置字体颜色白色  
      OLED.setTextSize(2); 
      OLED.setCursor(0,0);
      OLED.print("B");
      OLED.setCursor(18,0);
      OLED.print("P");
      OLED.setCursor(36,0);
      OLED.print("M");
      if(beatAvg<100)
        OLED.setCursor(70,0);
      else
        OLED.setCursor(55,0);  
      OLED.setTextSize(4);
      if(beatAvg >= 60)
        OLED.print(beatAvg);
      else
        OLED.drawBitmap(70, 0, str, 40, 30, WHITE);
      OLED.setCursor(0,18);
      OLED.setTextSize(1);
      OLED.print(beatsPerMinute); 
      OLED.println(" bp");  
      OLED.setCursor(0,28);
      OLED.print(delta);
      OLED.println(" ms"); 
      Serial1.print("A");     
      Serial1.print(beatAvg);
      for (int i = 0; i < SAMP_SIZE; i++) {
        //OLED.drawFastVLine(i, 31 - (rates[(i + rateSpot) % SAMP_SIZE] / tmp), 1, WHITE);
        OLED.drawLine(i, 55 - (rates[(i + rateSpot) % SAMP_SIZE] / tmp)*2, i+1, 55 - (rates[(i + rateSpot) % SAMP_SIZE+1] / tmp)*2, WHITE);
      }

      OLED.display();
    }     
  }

  if (irValue < 50000) {
    reset();
    zero();
    seen = 0;
    OLED.setCursor(2,0);
    OLED.print("Put Finger On Sensor");  //把手指放在感应器上。
    OLED.drawBitmap(44, 20, str, 36, 30, WHITE);
    OLED.drawLine(0,36,128,36, WHITE);
    OLED.display();
    Serial1.print("B");
  }
  key_nc = key;
}

void reset() {
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(WHITE);
  OLED.setCursor(0,0);
}

void zero() { 
  for (int i = 0; i < SAMP_SIZE; i++) {
    rates[i] = 0;
  }
}

void QR_cude(const char *QR_buff){ 
  OLED.clearDisplay();
  QRCode qrcode;                      //定义QR对象
  uint8_t qrcodeData[qrcode_getBufferSize(1)];           //大小跟着版本变化
  qrcode_initText(&qrcode, qrcodeData, 1, 0, QR_buff);   // 版本1-40/3  容错0-3/0
  for (uint8 y = 0; y < qrcode.size; y++){
    for (uint8 x = 0; x < qrcode.size; x++){
        if (qrcode_getModule(&qrcode, x, y))
          OLED.fillRoundRect(x*3+33, y*3, 3, 3, 0, BLACK); //坐标x  坐标y  长  宽  圆角
        else
          OLED.fillRoundRect(x*3+33, y*3, 3, 3, 0, WHITE);
       }
    }
    OLED.display();
}

