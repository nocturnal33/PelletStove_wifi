#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Web interface password
const char* webPassword = "SetPasswordHere";

// Static IP configuration
// use IP address: x, x, x, x
IPAddress local_IP(192, 168, 5, 100);
IPAddress gateway(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(1, 1, 1, 1);

// Servo setup
Servo myServo;
const int servoPin = 18;

// Web server on port 80
WebServer server(80);

// Current Heat level (0-8)
int HeatLevel = 0;

// Session management
String sessionToken = "";
const unsigned long sessionTimeout = 3600000; // 1 hour in milliseconds
unsigned long lastActivity = 0;

// Generate a simple session token
String generateToken() {
  String token = "";
  for (int i = 0; i < 32; i++) {
    token += String(random(0, 16), HEX);
  }
  return token;
}

// Login page HTML
const char loginPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Login - Heat Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 100px;
      background-color: #f0f0f0;
    }
    .login-container {
      background: white;
      padding: 40px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      max-width: 300px;
      margin: 0 auto;
    }
    h1 {
      color: #333;
      margin-bottom: 30px;
    }
    input[type="password"] {
      width: 100%;
      padding: 12px;
      margin: 10px 0;
      border: 2px solid #ddd;
      border-radius: 5px;
      box-sizing: border-box;
      font-size: 16px;
    }
    button {
      width: 100%;
      padding: 12px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
      margin-top: 10px;
    }
    button:hover {
      background-color: #45a049;
    }
    .error {
      color: red;
      margin-top: 10px;
      display: none;
    }
  </style>
</head>
<body>
  <div class="login-container">
    <h1>Heat Control</h1>
    <form id="loginForm">
      <input type="password" id="password" placeholder="Enter Password" required autofocus>
      <button type="submit">Login</button>
      <div class="error" id="errorMsg">Incorrect password!</div>
    </form>
  </div>

  <script>
    document.getElementById('loginForm').onsubmit = function(e) {
      e.preventDefault();
      var pass = document.getElementById('password').value;
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            // Get the token from the response
            var token = this.responseText;
            console.log("Login successful, token: " + token);
            // Try cookie first, then fallback to URL parameter
            window.location.href = "/?token=" + encodeURIComponent(token);
          } else {
            document.getElementById('errorMsg').style.display = 'block';
            document.getElementById('password').value = '';
            document.getElementById('password').focus();
          }
        }
      };
      xhr.open("POST", "/login", true);
      xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
      xhr.send("password=" + encodeURIComponent(pass));
    }
  </script>
</body>
</html>
)=====";

// Main control page HTML
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
    h3{
      color: #ff3300;
      font-size: 20px;
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
    .logout-btn {
      margin-top: 60px;
      padding: 10px 30px;
      background-color: #f44336;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      font-size: 14px;
    }
    .logout-btn:hover {
      background-color: #da190b;
    }
  </style>
</head>
<body>
  <h1>Heat Control</h1>
  <h3>PelPro 130</h3>
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
      <span onclick="setHeat(8)">8 (HIGH)</span>
    </div>
  </div>
  <button class="logout-btn" onclick="logout()">Logout</button>

  <script>
    var slider = document.getElementById("HeatSlider");
    var display = document.getElementById("HeatDisplay");
    
    // Get token from URL parameters
    function getToken() {
      var urlParams = new URLSearchParams(window.location.search);
      return urlParams.get('token');
    }
    
    // Set Heat when clicking on labels
    function setHeat(level) {
      slider.value = level;
      display.innerHTML = level;
      
      var token = getToken();
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setHeat?level=" + level + "&token=" + token, true);
      xhr.send();
    }
    
    slider.oninput = function() {
      display.innerHTML = this.value;
    }
    
    slider.onchange = function() {
      var token = getToken();
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setHeat?level=" + this.value + "&token=" + token, true);
      xhr.send();
    }
    
    window.onload = function() {
      var token = getToken();
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          slider.value = this.responseText;
          display.innerHTML = this.responseText;
        }
      };
      xhr.open("GET", "/getHeat?token=" + token, true);
      xhr.send();
    }
    
    function logout() {
      document.cookie = "session=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
      window.location.href = "/login";
    }
  </script>
</body>
</html>
)=====";

// Check if user is authenticated
bool isAuthenticated() {
  // First check for session cookie
  if (server.hasHeader("Cookie")) {
    String cookie = server.header("Cookie");
    Serial.println("Cookie received: " + cookie);
    
    int tokenIndex = cookie.indexOf("session=");
    if (tokenIndex != -1) {
      String clientToken = cookie.substring(tokenIndex + 8);
      int endIndex = clientToken.indexOf(';');
      if (endIndex != -1) {
        clientToken = clientToken.substring(0, endIndex);
      }
      
      Serial.println("Client token from cookie: " + clientToken);
      Serial.println("Server token: " + sessionToken);
      
      // Check if token matches and hasn't expired
      if (clientToken == sessionToken && 
          (millis() - lastActivity < sessionTimeout)) {
        lastActivity = millis(); // Refresh activity time
        Serial.println("Authentication successful via cookie!");
        return true;
      } else {
        Serial.println("Token mismatch or expired");
      }
    } else {
      Serial.println("No session token found in cookie");
    }
  } else {
    Serial.println("No cookie header found");
  }
  
  // Fallback: Check for token in URL parameters
  if (server.hasArg("token")) {
    String urlToken = server.arg("token");
    Serial.println("Token from URL: " + urlToken);
    Serial.println("Server token: " + sessionToken);
    
    if (urlToken == sessionToken && 
        (millis() - lastActivity < sessionTimeout)) {
      lastActivity = millis(); // Refresh activity time
      Serial.println("Authentication successful via URL token!");
      return true;
    } else {
      Serial.println("URL token mismatch or expired");
    }
  }
  
  return false;
}

void handleLogin() {
  server.send(200, "text/html", loginPage);
}

void handleLoginPost() {
  Serial.println("=== LOGIN POST REQUEST ===");
  
  if (server.hasArg("password")) {
    String pass = server.arg("password");
    
    Serial.print("Login attempt with password: ");
    Serial.println(pass);
    Serial.print("Expected password: ");
    Serial.println(webPassword);
    Serial.print("Password match: ");
    Serial.println(pass == webPassword ? "YES" : "NO");
    
    if (pass == webPassword) {
      sessionToken = generateToken();
      lastActivity = millis();
      Serial.println("Login successful! Token: " + sessionToken);
      Serial.println("Session timeout: " + String(sessionTimeout) + "ms");
      
      // Set the session cookie in the response
      String cookieHeader = "session=" + sessionToken + "; Path=/; Max-Age=3600";
      Serial.println("Setting cookie header: " + cookieHeader);
      server.sendHeader("Set-Cookie", cookieHeader);
      server.sendHeader("Access-Control-Allow-Credentials", "true");
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "text/plain", sessionToken);
    } else {
      Serial.println("Login failed - incorrect password");
      server.send(401, "text/plain", "Unauthorized");
    }
  } else {
    Serial.println("Login failed - no password provided");
    server.send(400, "text/plain", "Bad Request");
  }
  Serial.println("=== END LOGIN POST ===");
}

void handleRoot() {
  Serial.println("=== ROOT REQUEST ===");
  Serial.println("Checking authentication...");
  
  // Debug: Print all headers received
  Serial.println("All headers received:");
  for (int i = 0; i < server.headers(); i++) {
    Serial.println("Header " + String(i) + ": " + server.headerName(i) + " = " + server.header(i));
  }
  
  if (isAuthenticated()) {
    Serial.println("User is authenticated, serving main page");
    server.sendHeader("Access-Control-Allow-Credentials", "true");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", webpage);
  } else {
    Serial.println("User not authenticated, redirecting to login");
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "Redirecting to login");
  }
  Serial.println("=== END ROOT REQUEST ===");
}

void handleSetHeat() {
  if (!isAuthenticated()) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }
  
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
  if (!isAuthenticated()) {
    server.send(401, "text/plain", "Unauthorized");
    return;
  }
  server.send(200, "text/plain", String(HeatLevel));
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0)); // For token generation
  
  // Initialize servo
  myServo.attach(servoPin);
  myServo.write(0);
  
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
  Serial.print("Web password is: ");
  Serial.println(webPassword);
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/login", HTTP_POST, handleLoginPost);
  server.on("/setHeat", handleSetHeat);
  server.on("/getHeat", handleGetHeat);
  
  server.begin();
  Serial.println("Web server started!");
  Serial.println("Access at: "); 
  Serial.print(HeatLevel);
  Serial.println("");
}

void loop() {
  server.handleClient();
}
