/*
 * ESP32 Bluetooth Controller for STM32 Micromouse Car
 * Receives Bluetooth commands and forwards to STM32 via UART
 * 
 * Hardware Connections:
 * ESP32 TX (GPIO17) -> STM32 RX (PA10)
 * ESP32 RX (GPIO16) -> STM32 TX (PA9)
 * ESP32 GND -> STM32 GND
 * 
 * Bluetooth Commands:
 * F - Forward
 * B - Backward
 * L - Turn Left
 * R - Turn Right
 * Q - Rotate Left
 * E - Rotate Right
 * S - Stop
 * + - Speed Up
 * - - Speed Down
 */

#include "BluetoothSerial.h"

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

// Create Bluetooth Serial object
BluetoothSerial SerialBT;

// UART to STM32 (Hardware Serial 2)
#define STM32_SERIAL Serial2
#define STM32_RX_PIN 16  // ESP32 RX <- STM32 TX
#define STM32_TX_PIN 17  // ESP32 TX -> STM32 RX
#define STM32_BAUD 9600

// LED for status indication
#define LED_PIN 2  // Built-in LED on most ESP32 boards

// Variables
String btDeviceName = "STM32_Car";
unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 2000; // Stop car if no command for 2 seconds

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32 Bluetooth Controller ===");
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize UART to STM32
  STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  Serial.println("UART to STM32 initialized");
  
  // Initialize Bluetooth
  if(!SerialBT.begin(btDeviceName)) {
    Serial.println("Bluetooth initialization failed!");
    while(1) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(200);
    }
  }
  
  Serial.println("Bluetooth initialized successfully");
  Serial.print("Device name: ");
  Serial.println(btDeviceName);
  Serial.println("Ready to pair! Search for: " + btDeviceName);
  Serial.println("\nCommands:");
  Serial.println("F - Forward");
  Serial.println("B - Backward");
  Serial.println("L - Turn Left");
  Serial.println("R - Turn Right");
  Serial.println("Q - Rotate Left");
  Serial.println("E - Rotate Right");
  Serial.println("S - Stop");
  Serial.println("+ - Speed Up");
  Serial.println("- - Speed Down");
  
  // Blink LED to show ready
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}

void loop() {
  // Check for Bluetooth commands
  if (SerialBT.available()) {
    char command = SerialBT.read();
    
    // Echo command back to Bluetooth
    SerialBT.print("Command: ");
    SerialBT.println(command);
    
    // Forward command to STM32
    STM32_SERIAL.write(command);
    
    // Print to Serial Monitor
    Serial.print("BT -> STM32: ");
    Serial.println(command);
    
    // Update last command time
    lastCommandTime = millis();
    
    // Blink LED on command
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
  }
  
  // Check for response from STM32 (optional)
  if (STM32_SERIAL.available()) {
    String response = STM32_SERIAL.readStringUntil('\n');
    SerialBT.println(response);
    Serial.print("STM32 -> BT: ");
    Serial.println(response);
  }
  
  // Check for Serial Monitor commands (for testing)
  if (Serial.available()) {
    char command = Serial.read();
    STM32_SERIAL.write(command);
    Serial.print("Serial -> STM32: ");
    Serial.println(command);
  }
  
  // Safety: Stop car if no command received for a while
  if (millis() - lastCommandTime > COMMAND_TIMEOUT && lastCommandTime != 0) {
    STM32_SERIAL.write('S');  // Send stop command
    SerialBT.println("Auto-stop: No command timeout");
    Serial.println("Auto-stop: No command timeout");
    lastCommandTime = 0;  // Reset to avoid repeated stops
  }
  
  // LED indicates Bluetooth connection status
  if (SerialBT.hasClient()) {
    // Slow blink when connected
    static unsigned long ledTimer = 0;
    if (millis() - ledTimer > 1000) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      ledTimer = millis();
    }
  } else {
    // Fast blink when not connected
    static unsigned long ledTimer = 0;
    if (millis() - ledTimer > 200) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      ledTimer = millis();
    }
  }
  
  delay(10);  // Small delay to prevent CPU overload
}

// Optional: Bluetooth event callback
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client connected!");
    SerialBT.println("Connected to STM32 Car Controller");
    SerialBT.println("Send commands to control the car");
  }
  
  if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Client disconnected!");
    // Send stop command when disconnected
    STM32_SERIAL.write('S');
  }
}
