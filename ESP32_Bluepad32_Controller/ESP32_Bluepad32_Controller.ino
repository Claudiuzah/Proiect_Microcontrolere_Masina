/*
 * ESP32 Bluepad32 PS4 Controller for STM32 Micromouse
 * SIMPLIFIED VERSION - Only PS4 controller, no keyboard
 * 
 * Hardware:
 * ESP32 TX (GPIO17) -> STM32 PA10 (USART1 RX)
 * ESP32 RX (GPIO16) -> STM32 PA9  (USART1 TX)
 * ESP32 GND -> STM32 GND
 * 
 * Commands sent to STM32:
 * '1' = Forward        (R2 trigger)
 * '2' = Backward       (L2 trigger)
 * '3' = Turn Left      (Left stick left)
 * '4' = Turn Right     (Left stick right)
 * '5' = Rotate Left    (L1 button)
 * '6' = Rotate Right   (R1 button)
 * '7' = PWM+           (Triangle)
 * '8' = PWM-           (Cross)
 * '0' = Stop           (Circle / auto-release)
 */

#include <Bluepad32.h>

// UART to STM32 - Using GPIO21/22 (Serial2 GPIO16/17 conflicts with Bluepad32)
#define STM32_TX_PIN 21
#define STM32_RX_PIN 22
#define STM32_BAUD 9600

HardwareSerial STM32_SERIAL(1);  // Use UART1 instead of UART2

// LED indicator
#define LED_PIN 2

// Control parameters
bool manualControl = false;
unsigned long lastCommandTime = 0;
const int DEADZONE = 250;      // Joystick deadzone (out of 512) - HIGH for stick drift

// Last sent command (avoid spamming)
char lastCommand = '0';

// Speed button timing (allow repeated presses)
unsigned long lastSpeedButtonTime = 0;
const unsigned long SPEED_BUTTON_DELAY = 200;  // 200ms between speed changes

// Controller pointer
ControllerPtr myController = nullptr;

void sendToSTM32(char cmd) {
  if (cmd != lastCommand) {
    Serial.print("Sending: ");
    Serial.println(cmd);
    STM32_SERIAL.write(cmd);
    lastCommand = cmd;
    
    // Blink LED
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    
    lastCommandTime = millis();
  }
}

// Called when a controller connects
void onConnectedController(ControllerPtr ctl) {
  if (myController == nullptr) {
    Serial.println("CALLBACK: Controller connected!");
    myController = ctl;
    
    // LED feedback
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    
    // Stop motors on connect
    sendToSTM32('0');
  } else {
    Serial.println("CALLBACK: Controller already connected, ignoring new one");
  }
}

// Called when a controller disconnects
void onDisconnectedController(ControllerPtr ctl) {
  if (myController == ctl) {
    Serial.println("CALLBACK: Controller disconnected!");
    myController = nullptr;
    
    // Stop motors on disconnect
    sendToSTM32('0');
    lastCommand = '0';
  }
}

// Process controller inputs
void processController(ControllerPtr ctl) {
  // Get trigger values (0-1023, 1023 = fully pressed)
  // NOTE: brake() = L2, throttle() = R2 in Bluepad32
  int r2Value = ctl->throttle();  // R2 trigger (throttle)
  int l2Value = ctl->brake();     // L2 trigger (brake)
  
  // Get joystick values (-512 to 511)
  int leftStickX = ctl->axisX();    // Left stick X
  int rightStickX = ctl->axisRX();  // Right stick X
  
  // DEBUG: Print values when something is pressed
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 1000) {
    if (r2Value > 100 || l2Value > 100 || abs(leftStickX) > DEADZONE || abs(rightStickX) > DEADZONE) {
      Serial.printf("R2=%d L2=%d LeftX=%d RightX=%d\n", r2Value, l2Value, leftStickX, rightStickX);
      lastDebugPrint = millis();
    }
  }
  
  // Button states
  bool circle = ctl->b();        // Circle (B on Xbox)
  bool triangle = ctl->y();      // Triangle (Y on Xbox)
  bool cross = ctl->a();         // Cross/X (A on Xbox)
  bool l1 = ctl->l1();           // L1
  bool r1 = ctl->r1();           // R1
  
  // Circle button - Emergency Stop
  if (circle) {
    sendToSTM32('0');
    manualControl = false;
    return;
  }
  
  // Triangle - PWM+ (Speed Up) - Allow repeated presses
  if (triangle) {
    if (millis() - lastSpeedButtonTime > SPEED_BUTTON_DELAY) {
      STM32_SERIAL.write('7');
      Serial.println("Speed UP");
      lastSpeedButtonTime = millis();
    }
    return;
  }
  
  // Cross (X) button - PWM- (Speed Down) - Allow repeated presses
  if (cross) {
    if (millis() - lastSpeedButtonTime > SPEED_BUTTON_DELAY) {
      STM32_SERIAL.write('8');
      Serial.println("Speed DOWN");
      lastSpeedButtonTime = millis();
    }
    return;
  }

  
  // Micromouse simple controls
  // Priority order: buttons > triggers > sticks
  
  // R2 = Forward
  if (r2Value > 300) {
    sendToSTM32('1');
    manualControl = true;
    return;
  }
  
  // L2 = Backward
  if (l2Value > 300) {
    sendToSTM32('2');
    manualControl = true;
    return;
  }
  
  // L1 button = Rotate in place LEFT
  if (l1) {
    sendToSTM32('5');
    manualControl = true;
    return;
  }
  
  // R1 button = Rotate in place RIGHT
  if (r1) {
    sendToSTM32('6');
    manualControl = true;
    return;
  }
  
  // Left stick = Turn left/right
  if (rightStickX < -DEADZONE) {
    sendToSTM32('3');
    manualControl = true;
    return;
  }
  
  if (rightStickX > DEADZONE) {
    sendToSTM32('4');
    manualControl = true;
    return;
  }
  
  // All controls released - auto stop
  if (manualControl) {
    sendToSTM32('0');
    manualControl = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n========================================");
  Serial.println("ESP32 Bluepad32 Controller for STM32 Car");
  Serial.println("========================================\n");
  
  // Setup LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[1] LED initialized");
  
  // Setup UART to STM32 - IMPORTANT: TX and RX pins explicitly set
  STM32_SERIAL.begin(STM32_BAUD, SERIAL_8N1, STM32_RX_PIN, STM32_TX_PIN);
  delay(100);
  
  Serial.println("[2] UART initialized:");
  Serial.println("    TX: GPIO21 -> STM32 PA10 (USART1 RX)");
  Serial.println("    RX: GPIO22 <- STM32 PA9 (USART1 TX)");
  Serial.print("    Baud: ");
  Serial.println(STM32_BAUD);
  
  // Initialize Bluepad32
  Serial.println("\n[3] Initializing Bluepad32...");
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();  // Clear paired devices on startup
  
  Serial.println("\n========================================");
  Serial.println("READY!");
  Serial.println("========================================");
  Serial.println("Waiting for controller...");
  Serial.println("\nPAIRING INSTRUCTIONS:");
  Serial.println("1. Hold PS + Share buttons for 3 seconds");
  Serial.println("2. Controller will flash, then connect");
  Serial.println("3. No MAC address configuration needed!");
  Serial.println("========================================\n");
  
  // LED test
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  // Update Bluepad32
  BP32.update();
  
  // Process connected controller
  if (myController && myController->isConnected()) {
    processController(myController);
  }
  
  // Auto-stop if no command for timeout period
  if (manualControl && millis() - lastCommandTime > 500) {
    sendToSTM32('X');
    manualControl = false;
  }
  
  // Print battery level every 10 seconds
  static unsigned long lastBatteryPrint = 0;
  if (myController && myController->isConnected() && millis() - lastBatteryPrint > 10000) {
    Serial.print("Controller Battery: ");
    Serial.print(myController->battery());
    Serial.println("/255");
    lastBatteryPrint = millis();
  }
  
  delay(10);
}
