/*
 * ESP32 WiFi Controller for STM32 Car - Optimized Version
 * Minimal HTML for small flash size
 */

#include <WiFi.h>
#include <WebServer.h>

// WiFi Access Point credentials
const char* ap_ssid = "STM32_Car";
const char* ap_password = "12345678";

// Web server
WebServer server(80);

// UART to STM32
#define STM32_SERIAL Serial2
#define STM32_RX_PIN 16  // ESP32 GPIO16 (RX) ← STM32 PA9 (TX)
#define STM32_TX_PIN 17  // ESP32 GPIO17 (TX) → STM32 PA10 (RX)
#define STM32_BAUD 9600

#define LED_PIN 2

unsigned long lastCommandTime = 0;

void sendToSTM32(char cmd) {
  Serial.print(">>> Sending to STM32: ");
  Serial.println(cmd);
  STM32_SERIAL.write(cmd);
  lastCommandTime = millis();
  
  // Blink LED
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}

// Minimal HTML page
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width, initial-scale=1">
<style>body{text-align:center;font-family:Arial;background:#667eea;color:#fff;padding:20px}
button{width:80px;height:80px;margin:5px;font-size:16px;border:none;border-radius:10px;background:#fff;cursor:pointer}
.stop{background:#ff4757;color:#fff;width:170px}.speed{width:50px;height:50px;background:#feca57}</style></head>
<body><h1>STM32 Car</h1><div><button onclick="c('W')">FWD</button></div>
<div><button onclick="c('A')">LEFT</button><button class="stop" onclick="c('X')">STOP</button>
<button onclick="c('D')">RIGHT</button></div><div><button onclick="c('Q')">ROT L</button>
<button onclick="c('S')">BACK</button><button onclick="c('E')">ROT R</button></div>
<div style="margin-top:20px"><button class="speed" onclick="c('%2B')">+</button>
<button class="speed" onclick="c('-')">-</button></div>
<p id="s">Ready</p><script>function c(x){fetch('/c?v='+x).then(r=>r.text()).then(d=>
document.getElementById('s').innerText=d)}</script></body></html>
)rawliteral";

void handleRoot() {
  Serial.println(">>> Web page requested");
  server.send_P(200, "text/html", htmlPage);
}

void handleCmd() {
  Serial.print(">>> Command received: ");
  if (server.hasArg("v")) {
    String cmdStr = server.arg("v");
    if(cmdStr.length() > 0) {
      char cmd = cmdStr.charAt(0);
      
      // Handle URL-encoded plus sign (%2B becomes space, so we check for space)
      if(cmdStr == "%2B" || cmd == ' ') {
        cmd = '+';
      }
      
      Serial.println(cmd);
      
      // Validate command - now includes WASD keys
      if(cmd == 'W' || cmd == 'A' || cmd == 'S' || cmd == 'D' || cmd == 'X' ||
         cmd == 'F' || cmd == 'B' || cmd == 'L' || cmd == 'R' || 
         cmd == 'Q' || cmd == 'E' ||
         cmd == '+' || cmd == '-' ||
         cmd == 'w' || cmd == 'a' || cmd == 's' || cmd == 'd' || cmd == 'x' ||
         cmd == 'f' || cmd == 'b' || cmd == 'l' || cmd == 'r' || 
         cmd == 'q' || cmd == 'e') {
        sendToSTM32(cmd);
        String msg = "OK: " + String(cmd);
        server.send(200, "text/plain", msg);
      } else {
        Serial.print("INVALID command: ");
        Serial.println((int)cmd);
        server.send(400, "text/plain", "Invalid command");
      }
    } else {
      Serial.println("EMPTY command");
      server.send(400, "text/plain", "Empty command");
    }
  } else {
    Serial.println("ERROR - No argument");
    server.send(400, "text/plain", "Error");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("ESP32 WiFi Controller for STM32 Car");
  Serial.println("========================================\n");
  
  // LED setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[1] LED initialized on GPIO2");
  
  // UART to STM32
  STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  Serial.println("[2] UART initialized:");
  Serial.println("    GPIO16 (RX) <- STM32 PA9 (TX)");
  Serial.println("    GPIO17 (TX) -> STM32 PA10 (RX)");
  Serial.println("    Baud: 9600");
  
  // WiFi AP
  Serial.println("\n[3] Starting WiFi Access Point...");
  WiFi.softAP(ap_ssid, ap_password);
  delay(100);
  Serial.print("    SSID: ");
  Serial.println(ap_ssid);
  Serial.print("    Password: ");
  Serial.println(ap_password);
  Serial.print("    IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Web server
  Serial.println("\n[4] Starting Web Server...");
  server.on("/", handleRoot);
  server.on("/c", handleCmd);
  server.begin();
  Serial.println("    Server running on port 80");
  
  // Test LED
  Serial.println("\n[5] Testing LED...");
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  // Test UART connection - send test character to STM32
  Serial.println("\n[5] Testing UART connection...");
  Serial.println("    Sending 'T' to STM32 (full speed test)");
  STM32_SERIAL.write('T');
  delay(500);
  STM32_SERIAL.write('X');  // Stop after test
  Serial.println("    Test complete!");
  
  Serial.println("\n========================================");
  Serial.println("READY!");
  Serial.println("========================================");
  Serial.println("1. Connect to WiFi: STM32_Car");
  Serial.println("2. Open browser: http://192.168.4.1");
  Serial.println("3. Or type commands here (W/A/S/D/X)");
  Serial.println("========================================\n");
}

void loop() {
  server.handleClient();
  
  // Debug: Send test character every 3 seconds
  static unsigned long lastTest = 0;
  if(millis() - lastTest > 3000) {
    Serial.println(">>> DEBUG: Sending 'W' to test connection");
    STM32_SERIAL.write('W');
    STM32_SERIAL.flush();  // Make sure it's sent
    lastTest = millis();
  }
  
  // Auto-stop after 2 seconds
  if (millis() - lastCommandTime > 2000 && lastCommandTime != 0) {
    Serial.println(">>> Auto-stop timeout");
    sendToSTM32('X');
    lastCommandTime = 0;
  }
  
  // STM32 response
  if (STM32_SERIAL.available()) {
    String response = STM32_SERIAL.readStringUntil('\n');
    Serial.print("<<< STM32: ");
    Serial.println(response);
  }
  
  // Serial commands for testing
  if (Serial.available()) {
    char cmd = Serial.read();
    if(cmd >= 32 && cmd <= 126) { // Printable characters only
      Serial.print(">>> Manual command: ");
      Serial.println(cmd);
      sendToSTM32(cmd);
    }
  }
  
  delay(10);
}
