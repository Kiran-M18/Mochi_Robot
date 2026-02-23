#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 1. Setup Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 2. Network Credentials
const char* ssid = "Red";
const char* password = "e2vmtm3g";

WebServer server(80);

// 3. HTML for the Website (Stored inside the robot!)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; background-color: #222; color: white; }
    .btn { background-color: #ff4d4d; border: none; color: white; padding: 15px 32px; 
           font-size: 20px; margin: 10px; border-radius: 12px; cursor: pointer; }
    .btn:active { background-color: #cc0000; }
  </style>
</head>
<body>
  <h1>Mochi Controller</h1>
  <button class="btn" onclick="fetch('/happy')">Happy Eyes</button>
  <button class="btn" onclick="fetch('/angry')">Angry Eyes</button>
  <button class="btn" onclick="fetch('/sleep')">Sleep</button>
</body>
</html>)rawliteral";

void drawEyes(String emotion) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  
  if(emotion == "happy") {
    // In reality, you would draw bitmaps here
    display.println("^^   ^^"); 
  } else if (emotion == "angry") {
    display.println(">>   <<");
  } else {
    display.println("--   --");
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  drawEyes("sleep");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define Web Routes
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });
  
  server.on("/happy", []() {
    drawEyes("happy");
    server.send(200, "text/plain", "OK");
  });

  server.on("/angry", []() {
    drawEyes("angry");
    server.send(200, "text/plain", "OK");
  });
    server.on("/sleep", []() {
    drawEyes("sleep");
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}