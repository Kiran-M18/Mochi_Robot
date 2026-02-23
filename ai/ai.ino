#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,-1);

#define TOUCH_PIN 2

const char* GEMINI_KEY="Your API Key";

unsigned long touchStart=0;
bool wifiConnected=false;


// ---------- OLED PRINT ----------
void show(String s,int size=2){
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println(s);
  display.display();
}


// ---------- GEMINI REQUEST ----------
String askGemini(String prompt){

  WiFiClientSecure client;
  client.setInsecure();

  Serial.println("Connecting Gemini...");

  if(!client.connect("generativelanguage.googleapis.com",443)){
    Serial.println("Gemini connect FAIL");
    return "network fail";
  }

  String url="/v1beta/models/gemini-1.5-flash:generateContent?key="+String(GEMINI_KEY);

  StaticJsonDocument<512> doc;
  JsonArray contents=doc.createNestedArray("contents");
  JsonObject obj=contents.createNestedObject();
  JsonArray parts=obj.createNestedArray("parts");
  parts.createNestedObject()["text"]=prompt;

  String body;
  serializeJson(doc,body);

  client.println("POST "+url+" HTTP/1.1");
  client.println("Host: generativelanguage.googleapis.com");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.println(body);

  // -------- WAIT RESPONSE (timeout safe) --------
  unsigned long timeout=millis();
  while(client.available()==0){
    if(millis()-timeout>15000){
      Serial.println("Gemini timeout");
      return "timeout";
    }
  }

  // -------- SKIP HEADERS --------
  while(client.connected()){
    String line=client.readStringUntil('\n');
    if(line=="\r") break;
  }

  // -------- READ BODY --------
  String payload=client.readString();

  Serial.println("Gemini response received");

  StaticJsonDocument<4096> resp;

  DeserializationError err=deserializeJson(resp,payload);

  if(err){
    Serial.println("JSON parse fail");
    return "AI parse error";
  }

  String out=resp["candidates"][0]["content"]["parts"][0]["text"]|"AI error";

  if(out.length()==0) out="empty response";

  return out;
}


// ---------- CONNECT OPEN WIFI ----------
void connectOpenWiFi(){

  show("Scanning",1);

  int n=WiFi.scanNetworks();

  for(int i=0;i<n;i++){

    if(WiFi.encryptionType(i)==WIFI_AUTH_OPEN){

      String ssid=WiFi.SSID(i);
      show("Connect:\n"+ssid,1);

      WiFi.begin(ssid.c_str());

      unsigned long start=millis();

      while(WiFi.status()!=WL_CONNECTED && millis()-start<10000)
        delay(300);

      if(WiFi.status()==WL_CONNECTED){

        show("Connected",1);
        delay(1500);

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0,0);
        display.println("IP:");
        display.println(WiFi.localIP());
        display.display();

        delay(3000);

        display.clearDisplay();
        display.display();

        wifiConnected=true;
        Serial.println("WIFI_OK");

        return;
      }
    }
  }

  show("No open WiFi",1);
}


// ---------- SETUP ----------
void setup(){

  Serial.begin(115200);
  delay(2000);

  pinMode(TOUCH_PIN,INPUT_PULLUP);

  Wire.begin(4,5);
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);

  show("WELCOME");
  delay(2000);

  show("Hold touch\n4 sec",1);
}


// ---------- LOOP ----------
void loop(){

  // WAIT TOUCH → CONNECT WIFI
  if(!wifiConnected){

    if(digitalRead(TOUCH_PIN)==LOW){

      if(touchStart==0) touchStart=millis();

      if(millis()-touchStart>4000)
        connectOpenWiFi();

    } else touchStart=0;

    return;
  }


  // -------- SERIAL CHAT --------
  if(Serial.available()){

    String q=Serial.readStringUntil('\n');
    q.trim();
    if(q=="") return;

    Serial.println("QUESTION:"+q);

    show("Thinking...",1);

    String reply=askGemini(q);

    // ⭐ ALWAYS SEND TO PYTHON
    Serial.println(reply);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(reply.substring(0,120));
    display.display();
  }
}