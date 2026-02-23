#include "BluetoothA2DPSink.h"

BluetoothA2DPSink a2dp_sink;

#define TOUCH_PIN 5   // D5 touch module output

unsigned long pressStart = 0;
unsigned long lastRelease = 0;

int tapCount = 0;
bool buttonPressed = false;
bool connected = true;
bool isPaused = false;

void setup(){

  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);

  // ---- YOUR I2S PINS ----
  i2s_pin_config_t my_pin_config = {
      .bck_io_num = 27,
      .ws_io_num = 26,
      .data_out_num = 25,
      .data_in_num = I2S_PIN_NO_CHANGE
  };

  a2dp_sink.set_pin_config(my_pin_config);
  a2dp_sink.start("ESP32 TOUCH SPEAKER");
}


void loop(){

  bool state = digitalRead(TOUCH_PIN);

  // ----- PRESS START -----
  if(state && !buttonPressed){
    buttonPressed = true;
    pressStart = millis();
  }

  // ----- RELEASE -----
  if(!state && buttonPressed){

    buttonPressed = false;

    unsigned long pressTime = millis() - pressStart;

    // ===== LONG PRESS =====
    if(pressTime > 1500){

      if(connected){
        Serial.println("DISCONNECT");
        a2dp_sink.disconnect();
        connected=false;
      }
      else{
        Serial.println("CONNECT");
        a2dp_sink.start("ESP32 TOUCH SPEAKER");
        connected=true;
      }

      tapCount = 0;
      delay(600);
      return;
    }

    // ===== SHORT TAP =====
    tapCount++;
    lastRelease = millis();
  }

  // ===== CHECK SINGLE / DOUBLE =====
  if(tapCount>0 && millis()-lastRelease>400){

    // ----- SINGLE TAP → PLAY / PAUSE -----
    if(tapCount==1){

      Serial.println("PLAY / PAUSE");

      if(isPaused){
        a2dp_sink.play();
      }else{
        a2dp_sink.pause();
      }

      isPaused=!isPaused;
    }

    // ----- DOUBLE TAP → NEXT -----
    if(tapCount>=2){
      Serial.println("NEXT TRACK");
      a2dp_sink.next();
    }

    tapCount=0;
  }
}
