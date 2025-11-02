/*
 * ESP32 WiFi + Bluetooth Controller for STM32 Micromouse Car
 * Supports both Bluetooth and WiFi HTTP control
 * 
 * Hardware Connections:
 * ESP32 TX (GPIO17) -> STM32 RX (PA10)
 * ESP32 RX (GPIO16) -> STM32 TX (PA9)
 * ESP32 GND -> STM32 GND
 */

#include "BluetoothSerial.h"
#include <WiFi.h>
#include <WebServer.h>

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

// WiFi credentials (Access Point mode - create your own WiFi network)
const char* ap_ssid = "STM32_Car_AP";
const char* ap_password = "12345678";  // Minimum 8 characters

// Or use Station mode to connect to existing WiFi
//#define USE_STATION_MODE
//const char* sta_ssid = "Your_WiFi_SSID";
//const char* sta_password = "Your_WiFi_Password";

// Create objects
BluetoothSerial SerialBT;
WebServer server(80);

// UART to STM32
#define STM32_SERIAL Serial2
#define STM32_RX_PIN 16
#define STM32_TX_PIN 17
#define STM32_BAUD 9600

// LED
#define LED_PIN 2

// Variables
String btDeviceName = "STM32_Car_BT";
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 2000;

// Function to send command to STM32
void sendToSTM32(char command) {
  STM32_SERIAL.write(command);
  lastCommandTime = millis();
  Serial.print("Command sent: ");
  Serial.println(command);
}

// HTML webpage
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>STM32 Car Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 20px;
    }
    h1 { margin-top: 50px; }
    .container {
      max-width: 400px;
      margin: 0 auto;
      background: rgba(255,255,255,0.1);
      border-radius: 20px;
      padding: 30px;
      backdrop-filter: blur(10px);
    }
    button {
      width: 100px;
      height: 100px;
      margin: 10px;
      font-size: 24px;
      border: none;
      border-radius: 15px;
      background: rgba(255,255,255,0.9);
      color: #333;
      cursor: pointer;
      box-shadow: 0 4px 15px rgba(0,0,0,0.2);
      transition: all 0.3s;
    }
    button:active {
      transform: scale(0.95);
      box-shadow: 0 2px 8px rgba(0,0,0,0.2);
    }
    .stop-btn {
      background: #ff4757;
      color: white;
      width: 220px;
      height: 80px;
      font-size: 32px;
      font-weight: bold;
    }
    .speed-btn {
      width: 60px;
      height: 60px;
      font-size: 32px;
      background: #feca57;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
      margin: 20px 0;
    }
    .info {
      background: rgba(255,255,255,0.2);
      padding: 15px;
      border-radius: 10px;
      margin: 20px 0;
    }
  </style>
</head>
<body>
  <h1>🚗 STM32 Car Controller</h1>
  <div class="container">
    <div class="info">
      <p>Use buttons or keyboard to control</p>
      <p>Arrow keys, WASD, or click buttons</p>
    </div>
    
    <div class="grid">
      <div></div>
      <button onclick="sendCmd('F')" id="btnF">⬆<br>FWD</button>
      <div></div>
      
      <button onclick="sendCmd('L')" id="btnL">⬅<br>LEFT</button>
      <button onclick="sendCmd('S')" class="stop-btn" id="btnS">⏹<br>STOP</button>
      <button onclick="sendCmd('R')" id="btnR">➡<br>RIGHT</button>
      
      <button onclick="sendCmd('Q')" id="btnQ">↶<br>ROT L</button>
      <button onclick="sendCmd('B')" id="btnB">⬇<br>BACK</button>
      <button onclick="sendCmd('E')" id="btnE">↷<br>ROT R</button>
    </div>
    
    <div style="margin-top: 30px;">
      <button onclick="sendCmd('+')" class="speed-btn">+</button>
      <span style="margin: 0 10px; font-size: 20px;">Speed</span>
      <button onclick="sendCmd('-')" class="speed-btn">-</button>
    </div>
    
    <div class="info" style="margin-top: 30px;">
      <p id="status">Ready</p>
    </div>
  </div>
  
  <script>
    function sendCmd(cmd) {
      fetch('/cmd?c=' + cmd)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = data;
          setTimeout(() => {
            document.getElementById('status').innerText = 'Ready';
          }, 1000);
        });
    }
    
    // Keyboard control
    document.addEventListener('keydown', function(event) {
      switch(event.key) {
        case 'ArrowUp': case 'w': case 'W':
          sendCmd('F'); break;
        case 'ArrowDown': case 's': case 'S':
          sendCmd('B'); break;
        case 'ArrowLeft': case 'a': case 'A':
          sendCmd('L'); break;
        case 'ArrowRight': case 'd': case 'D':
          sendCmd('R'); break;
        case 'q': case 'Q':
          sendCmd('Q'); break;
        case 'e': case 'E':
          sendCmd('E'); break;
        case ' ': // Spacebar
          sendCmd('S'); break;
        case '+': case '=':
          sendCmd('+'); break;
        case '-': case '_':
          sendCmd('-'); break;
      }
    });
    
    // Auto-refresh connection
    setInterval(() => {
      fetch('/ping').catch(() => {
        document.getElementById('status').innerText = 'Connection lost!';
      });
    }, 5000);
  </script>
</body>
</html>
)rawliteral";

// HTTP handlers
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleCommand() {
  if (server.hasArg("c")) {
    String cmd = server.arg("c");
    if (cmd.length() > 0) {
      char command = cmd.charAt(0);
      sendToSTM32(command);
      server.send(200, "text/plain", "Command: " + String(command));
    }
  } else {
    server.send(400, "text/plain", "No command");
  }
}

void handlePing() {
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32 WiFi + Bluetooth Controller ===");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize UART to STM32
  STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  Serial.println("UART initialized");
  
  // Initialize Bluetooth
  SerialBT.begin(btDeviceName);
  Serial.println("Bluetooth: " + btDeviceName);
  
  // Initialize WiFi
#ifdef USE_STATION_MODE
  WiFi.begin(sta_ssid, sta_password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
#else
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("WiFi AP started");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
#endif
  
  // Setup web server
  server.on("/", handleRoot);
  server.on("/cmd", handleCommand);
  server.on("/ping", handlePing);
  server.begin();
  Serial.println("Web server started");
  
  Serial.println("\n=== Ready! ===");
  Serial.println("Bluetooth: Pair with " + btDeviceName);
  Serial.println("WiFi: Connect to " + String(ap_ssid) + " and open http://" + WiFi.softAPIP().toString());
}

void loop() {
  // Handle web server
  server.handleClient();
  
  // Handle Bluetooth
  if (SerialBT.available()) {
    char command = SerialBT.read();
    SerialBT.print("Cmd: ");
    SerialBT.println(command);
    sendToSTM32(command);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  }
  
  // Handle STM32 response
  if (STM32_SERIAL.available()) {
    String response = STM32_SERIAL.readStringUntil('\n');
    SerialBT.println(response);
    Serial.println("STM32: " + response);
  }
  
  // Handle Serial Monitor
  if (Serial.available()) {
    char command = Serial.read();
    sendToSTM32(command);
  }
  
  // Auto-stop timeout
  if (millis() - lastCommandTime > COMMAND_TIMEOUT && lastCommandTime != 0) {
    sendToSTM32('S');
    SerialBT.println("Auto-stop");
    lastCommandTime = 0;
  }
  
  delay(10);
}
