#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22

#define TOUCH_PIN 5
#define BUZZER_PIN 25
#define BUZZ_CH 0

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- ROBOT STATE ----------
enum Mode {NORMAL, MESSAGE, LAUGH};
Mode mode=NORMAL;

unsigned long msgTimer=0;

// ---------- TAP ENGINE ----------
bool lastTouch=false;
unsigned long pressStart=0;
unsigned long lastTap=0;
int taps=0;
int rapidCounter=0;

// ---------- EYE ----------
float px=0,py=0;
float tx=0,ty=0;
float bounce=0;

// ---------- HELPERS ----------
void smooth(float &v,float t,float s){ v+=(t-v)*s; }

void beep(int f,int d){
  ledcWriteTone(BUZZ_CH,f);
  delay(d);
  ledcWriteTone(BUZZ_CH,0);
}

// ---------- SAFE MESSAGE DRAW ----------
void showMsg(const char* a,const char* b){

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0,10);
  display.println(a);

  display.setTextSize(1);
  display.setCursor(0,45);
  display.println(b);

  display.display();

  msgTimer=millis();
  mode=MESSAGE;
}

// ---------- DRAW FACE ----------
void drawFace(){

  display.clearDisplay();

  int cy=32+bounce;
  int lx=36,rx=92;

  display.drawCircle(lx,cy,18,WHITE);
  display.drawCircle(rx,cy,18,WHITE);

  display.fillCircle(lx+px,cy+py,6,WHITE);
  display.fillCircle(rx+px,cy+py,6,WHITE);

  display.display();
}

// ---------- SETUP ----------
void setup(){

  pinMode(TOUCH_PIN,INPUT_PULLUP);

  Wire.begin(SDA_PIN,SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);

  ledcSetup(BUZZ_CH,2000,8);
  ledcAttachPin(BUZZER_PIN,BUZZ_CH);

  display.clearDisplay();
  display.display();

  randomSeed(analogRead(34));
}

// ---------- LOOP ----------
void loop(){

  bool touch=digitalRead(TOUCH_PIN);

  // ===== TOUCH START =====
  if(touch && !lastTouch){
      pressStart=millis();
  }

  // ===== LONG PRESS =====
  if(touch && millis()-pressStart>2000){
      showMsg("OKAY OKAY","IM WORKING!");
      beep(1400,200);
      pressStart=millis()+999999;   // prevent repeat
  }

  // ===== TOUCH RELEASE =====
  if(!touch && lastTouch){

      // TAP DETECT
      if(millis()-lastTap<450) taps++;
      else taps=1;

      lastTap=millis();

      // RAPID TAP DETECT
      rapidCounter++;
      if(rapidCounter>=6){
          mode=LAUGH;
          msgTimer=millis();
          rapidCounter=0;
          beep(2000,150);
      }

      // DOUBLE TAP
      if(taps==2){
          showMsg("HEY!!!","BE PATIENT");
          beep(1800,150);
      }

      // TRIPLE TAP
      if(taps==3){
          showMsg("STOP","ANNOYING ME");
          beep(900,250);
          taps=0;
      }
  }

  lastTouch=touch;

  // ===== MESSAGE HOLD =====
  if(mode==MESSAGE){
      if(millis()-msgTimer>2000){
          mode=NORMAL;
      }
      delay(20);
      return;
  }

  // ===== LAUGH MODE =====
  if(mode==LAUGH){

      bounce=sin(millis()*0.03)*6;

      px=random(-2,3);
      py=random(-2,3);

      drawFace();

      if(millis()-msgTimer>3500){
          mode=NORMAL;
      }

      delay(16);
      return;
  }

  // ===== NORMAL ROBOT =====

  if(millis()%2500<20){
      tx=random(-6,7);
      ty=random(-4,5);
  }

  smooth(px,tx,0.08);
  smooth(py,ty,0.08);

  bounce=sin(millis()*0.004)*2;

  drawFace();

  delay(16);
}
