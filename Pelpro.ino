#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Static IP configuration
// format ip address as x, x, x, x
IPAddress local_IP();
IPAddress gateway();
IPAddress subnet();
IPAddress primaryDNS();
IPAddress secondaryDNS();

// Servo setup
Servo myServo;
const int servoPin = 18;

// LED setup
const int ledPin = 2; // Built-in LED on GPIO2
unsigned long lastLedBlink = 0;
bool ledState = false;

// Web server on port 80
WebServer server(80);

// Current Heat level (0-8)
int HeatLevel = 0;

// Control LED and Servo based on WiFi status
void updateWiFiLED() {
  static wl_status_t lastWifiStatus = WL_IDLE_STATUS;
  wl_status_t wifiStatus = WiFi.status();
  unsigned long currentTime = millis();
  
  // Check if WiFi was just disconnected
  if (lastWifiStatus == WL_CONNECTED && wifiStatus != WL_CONNECTED) {
    // WiFi just disconnected - reset servo to 0
    myServo.write(0);
    HeatLevel = 0;
    Serial.println("WiFi disconnected - Servo reset to 0");
  }
  
  lastWifiStatus = wifiStatus;
  
  switch (wifiStatus) {
    case WL_CONNECTED:
      // Solid LED when connected (active-low LED)
      digitalWrite(ledPin, LOW);
      break;
      
    case WL_IDLE_STATUS:
    case WL_NO_SSID_AVAIL:
    case WL_SCAN_COMPLETED:
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED:
      // Fast blink when disconnected or failed (250ms intervals)
      if (currentTime - lastLedBlink >= 250) {
        ledState = !ledState;
        digitalWrite(ledPin, ledState ? HIGH : LOW);
        lastLedBlink = currentTime;
      }
      break;
      
    default:
      // Slow blink for any other status (1 second intervals)
      if (currentTime - lastLedBlink >= 1000) {
        ledState = !ledState;
        digitalWrite(ledPin, ledState ? HIGH : LOW);
        lastLedBlink = currentTime;
      }
      break;
  }
}

// Main control page HTML (no password protection)
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Heat Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 50px;
      background-color: #f0f0f0;
    }
    h1 {
      color: #333;
      font-size: 30px;
      font-weight: bold;
      margin-bottom: 20px;
    }
    .slider-container {
      margin: 30px auto;
      width: 80%;
      max-width: 500px;
    }
    .slider {
      width: 100%;
      height: 75px;
      background: #f0f0f0;
      outline: none;
      opacity: 0.7;
      transition: opacity 0.2s;
      cursor: pointer;
      -webkit-appearance: none;
      appearance: none;
      border-radius: 10px;
    }
    
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 30px;
      height: 30px;
      background: #fff;
      cursor: pointer;
      border-radius: 50%;
      border: 3px solid #f44336;
      box-shadow: 0 2px 6px rgba(0,0,0,0.3);
    }
    
    .slider::-moz-range-thumb {
      width: 30px;
      height: 30px;
      background: #fff;
      cursor: pointer;
      border-radius: 50%;
      border: 3px solid #f44336;
      box-shadow: 0 2px 6px rgba(0,0,0,0.3);
    }
    
    .slider::-webkit-slider-track {
      height: 20px;
      background: #f44336;
      border-radius: 10px;
      border: none;
    }
    
    .slider::-webkit-slider-runnable-track {
      height: 20px;
      background: #f44336;
      border-radius: 10px;
      border: none;
    }
    
    .slider::-moz-range-track {
      height: 20px;
      background: #f44336;
      border-radius: 10px;
      border: none;
    }
    
    .slider:hover {
      opacity: 1;
    }
    .value-display {
      font-size: 48px;
      font-weight: bold;
      color: #4CAF50;
      margin: 20px;
    }
    .labels {
      display: flex;
      justify-content: space-between;
      margin-top: 20px;
      font-size: 14px;
      color: #666;
    }
    
    .labels span {
      cursor: pointer;
      padding: 5px 10px;
      border-radius: 3px;
      transition: background-color 0.2s;
    }
    
    .labels span:hover {
      background-color: rgba(244, 67, 54, 0.1);
    }
  </style>
</head>
<body>
  <h1>PelPro 130 - Heat Control</h1>
  <div class="value-display" id="HeatDisplay">0</div>
  <div class="slider-container">
    <input type="range" min="0" max="8" value="0" class="slider" id="HeatSlider">
    <div class="labels">
      <span onclick="setHeat(0)">OFF</span>
      <span onclick="setHeat(1)">1</span>
      <span onclick="setHeat(2)">2</span>
      <span onclick="setHeat(3)">3</span>
      <span onclick="setHeat(4)">4</span>
      <span onclick="setHeat(5)">5</span>
      <span onclick="setHeat(6)">6</span>
      <span onclick="setHeat(7)">7</span>
      <span onclick="setHeat(8)">8</span>
    </div>
  </div>

  <script>
    var slider = document.getElementById("HeatSlider");
    var display = document.getElementById("HeatDisplay");
    var isDragging = false; 

    slider.onmousedown = function() { isDragging = true; }
    slider.onmouseup = function() { isDragging = false; }
    slider.ontouchstart = function() { isDragging = true; }
    slider.ontouchend = function() { isDragging = false; }
    
    // Set Heat when clicking on labels
    function setHeat(level) {
      slider.value = level;
      display.innerHTML = level;
      
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setHeat?level=" + level, true);
      xhr.send();
    }
    
    slider.oninput = function() {
      display.innerHTML = this.value;
    }
    
    slider.onchange = function() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setHeat?level=" + this.value, true);
      xhr.send();
      isDragging = false;
    }

    function getHeatState() {
      // Only refresh if the user isn't actively dragging
      if (isDragging) {
        return;
      }
      
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          slider.value = this.responseText;
          display.innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "/getHeat", true);
      xhr.send();
    }
    
    window.onload = getHeatState;
    setInterval(getHeatState, 10000); // 10000ms = 10 seconds
  </script>
</body>
</html>
)=====";

void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleSetHeat() {
  if (server.hasArg("level")) {
    HeatLevel = server.arg("level").toInt();
    HeatLevel = constrain(HeatLevel, 0, 8);
    
    int servoAngle = map(HeatLevel, 0, 8, 0, 180);
    myServo.write(servoAngle);
    
    Serial.print("Heat set to: ");
    Serial.print(HeatLevel);
    Serial.print(" | Servo angle: ");
    Serial.println(servoAngle);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleGetHeat() {
  server.send(200, "text/plain", String(HeatLevel));
}

void setup() {
  Serial.begin(115200);
  
  // Initialize servo
  myServo.attach(servoPin);
  myServo.write(0);
  
  // Initialize LED (active-low)
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // Start with LED off (active-low)
  
  // Configure static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP configuration failed!");
  }
  
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes (no authentication needed)
  server.on("/", handleRoot);
  server.on("/setHeat", handleSetHeat);
  server.on("/getHeat", handleGetHeat);
  
  server.begin();
  Serial.println("Web server started!");
  Serial.println("No password protection - open access");
}

void loop() {
  server.handleClient();
  
  // Update LED based on WiFi status
  updateWiFiLED();
}

