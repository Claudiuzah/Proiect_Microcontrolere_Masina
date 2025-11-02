# Software Architecture Guide

## 🏗️ System Architecture

### High-Level Overview
```
┌─────────────────────────────────────────────────────┐
│                  User Interface Layer               │
│  ┌──────────────┐              ┌─────────────────┐ │
│  │ WiFi Web UI  │              │ UART Terminal   │ │
│  │ (ESP32)      │              │ (TeraTerm)      │ │
│  └──────┬───────┘              └────────┬────────┘ │
└─────────┼────────────────────────────────┼──────────┘
          │                                │
          ▼                                ▼
┌─────────────────────────────────────────────────────┐
│              Communication Layer                    │
│  ┌──────────────────────┐    ┌──────────────────┐ │
│  │ ESP32 Serial2 TX/RX  │    │ USART1 9600 baud │ │
│  └──────────┬───────────┘    └────────┬─────────┘ │
└─────────────┼────────────────────────┼─────────────┘
              │                        │
              └────────────┬───────────┘
                           ▼
┌─────────────────────────────────────────────────────┐
│              STM32 Application Layer                │
│  ┌──────────────────────────────────────────────┐  │
│  │         FreeRTOS Task Scheduler              │  │
│  │  ┌────────────────┐  ┌───────────────────┐  │  │
│  │  │ defaultTask    │  │ GUI_Task          │  │  │
│  │  │ - UART RX      │  │ - TouchGFX        │  │  │
│  │  │ - Command Proc │  │ - Display Update  │  │  │
│  │  │ - Motor Ctrl   │  │ - Touch Events    │  │  │
│  │  └────────┬───────┘  └───────────────────┘  │  │
│  └───────────┼──────────────────────────────────┘  │
└──────────────┼─────────────────────────────────────┘
               ▼
┌─────────────────────────────────────────────────────┐
│              Hardware Abstraction Layer             │
│  ┌──────────┐  ┌──────────┐  ┌─────────────────┐  │
│  │ TIM3 PWM │  │ GPIO     │  │ HAL_UART        │  │
│  │ (Motor)  │  │ (LED/Dir)│  │ (Communication) │  │
│  └────┬─────┘  └────┬─────┘  └─────────────────┘  │
└───────┼─────────────┼────────────────────────────────┘
        ▼             ▼
┌─────────────────────────────────────────────────────┐
│                  Hardware Layer                     │
│  ┌──────────────────────┐    ┌──────────────────┐  │
│  │  L9100S Motor Driver │    │  LEDs / Buttons  │  │
│  │  - Motor A (Left)    │    │  - LD3 (Green)   │  │
│  │  - Motor B (Right)   │    │  - LD4 (Red)     │  │
│  └──────────────────────┘    └──────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## 📂 Code Organization

### Core Files Structure

#### `Core/Inc/main.h`
**Purpose:** Global definitions, pin mappings, function prototypes

**Key Definitions:**
```c
// Motor control constants
#define MOTOR_MAX_SPEED 999
#define MOTOR_MIN_SPEED 0
#define MOTOR_A_PWM_CHANNEL TIM_CHANNEL_1  // PB4
#define MOTOR_B_PWM_CHANNEL TIM_CHANNEL_2  // PA7

// Pin definitions (auto-generated by CubeMX)
#define MOTOR_A_DIR_Pin GPIO_PIN_10
#define MOTOR_A_DIR_GPIO_Port GPIOC
#define MOTOR_B_DIR_Pin GPIO_PIN_11
#define MOTOR_B_DIR_GPIO_Port GPIOC
#define LD3_Pin GPIO_PIN_13
#define LD3_GPIO_Port GPIOG
#define LD4_Pin GPIO_PIN_14
#define LD4_GPIO_Port GPIOG
```

**Function Prototypes:**
```c
void Motor_Init(void);
void Motor_SetSpeed(char motor, int16_t speed);
void Motor_Stop(void);
void Motor_Forward(uint16_t speed);
void Motor_Backward(uint16_t speed);
void Motor_TurnLeft(uint16_t speed);
void Motor_TurnRight(uint16_t speed);
void Motor_RotateLeft(uint16_t speed);
void Motor_RotateRight(uint16_t speed);
```

#### `Core/Src/main.c`
**Purpose:** Main application logic, motor control implementation

**Key Sections:**

1. **Hardware Initialization** (lines ~180-210)
   - Peripheral initialization
   - Motor setup
   - UART configuration

2. **Printf Retargeting** (lines ~1130-1145)
   - `__io_putchar()` implementation
   - Routes printf to USART1

3. **Motor Control Functions** (lines ~1145-1330)
   - `Motor_Init()` - PWM startup
   - `Motor_SetSpeed()` - Core PWM/direction control
   - `Motor_Forward/Backward/Turn/Rotate()` - Movement primitives

4. **FreeRTOS Task** (lines ~1330-1420)
   - `StartDefaultTask()` - Main control loop
   - UART command reception
   - Command processing
   - Speed adjustment logic

#### `Core/Src/stm32f4xx_hal_msp.c`
**Purpose:** Hardware-specific peripheral initialization

**Key Functions:**
```c
HAL_UART_MspInit()    // Configure UART pins and clocks
HAL_TIM_MspPostInit() // Configure Timer PWM outputs
HAL_GPIO_Init()       // Initialize GPIO pins
```

#### `Core/Inc/FreeRTOSConfig.h`
**Purpose:** RTOS configuration

**Critical Settings:**
```c
#define configUSE_PREEMPTION              1
#define configCPU_CLOCK_HZ                180000000  // 180MHz
#define configTICK_RATE_HZ                1000       // 1ms tick
#define configMAX_PRIORITIES              7
#define configMINIMAL_STACK_SIZE          128
#define configTOTAL_HEAP_SIZE             15360
#define USE_NEWLIB_REENTRANT              1          // Thread-safe printf
```

## 🎯 Motor Control Algorithm

### PWM Generation

**Timer Configuration:**
```c
TIM3 Configuration:
- Clock: 90 MHz (APB1 x2)
- Prescaler: 89 (90MHz / 90 = 1MHz)
- Auto-reload (ARR): 999
- PWM Frequency: 1MHz / 1000 = 1kHz
- Resolution: 1000 steps (0-999)
```

**PWM Modes:**
- Channel 1 (PB4): PWM Mode 1
- Channel 2 (PA7): PWM Mode 1
- Output polarity: Active High

### L9100S Control Strategy

#### Forward Direction
```c
Motor A Forward:
  TIM3->CCR1 = pwm_value;        // PWM on IA1 (PB4)
  GPIOC->BSRR = GPIO_PIN_10 << 16; // LOW on IA2 (PC10)
  
Effective voltage: V_motor = (pwm_value / 999) * V_supply
```

#### Backward Direction
```c
Motor A Backward:
  TIM3->CCR1 = (999 - pwm_value);  // Inverted PWM on IA1
  GPIOC->BSRR = GPIO_PIN_10;         // HIGH on IA2
  
Creates voltage difference for reverse rotation
```

**Why inversion?**
- L9100S needs voltage difference between inputs
- Forward: IA1=HIGH(PWM), IA2=LOW → Vdiff = +PWM
- Backward: IA1=LOW(inv), IA2=HIGH → Vdiff = -PWM
- Inverted PWM maintains speed proportionality

### Movement Primitives

#### 1. Forward Motion
```c
void Motor_Forward(uint16_t speed) {
  Motor_SetSpeed('A', speed);   // Left motor forward
  Motor_SetSpeed('B', speed);   // Right motor forward
}
```
**Result:** Both wheels spin forward at same speed

#### 2. Backward Motion
```c
void Motor_Backward(uint16_t speed) {
  Motor_SetSpeed('A', -speed);  // Left motor backward
  Motor_SetSpeed('B', -speed);  // Right motor backward
}
```
**Result:** Both wheels spin backward

#### 3. Turn Left (Arc)
```c
void Motor_TurnLeft(uint16_t speed) {
  Motor_SetSpeed('A', speed / 2);  // Left slower
  Motor_SetSpeed('B', speed);      // Right faster
}
```
**Result:** Car follows left-curving arc

#### 4. Turn Right (Arc)
```c
void Motor_TurnRight(uint16_t speed) {
  Motor_SetSpeed('A', speed);      // Left faster
  Motor_SetSpeed('B', speed / 2);  // Right slower
}
```
**Result:** Car follows right-curving arc

#### 5. Rotate Left (Spin)
```c
void Motor_RotateLeft(uint16_t speed) {
  Motor_SetSpeed('A', -speed);  // Left backward
  Motor_SetSpeed('B', speed);   // Right forward
}
```
**Result:** Car spins counter-clockwise in place

#### 6. Rotate Right (Spin)
```c
void Motor_RotateRight(uint16_t speed) {
  Motor_SetSpeed('A', speed);   // Left forward
  Motor_SetSpeed('B', -speed);  // Right backward
}
```
**Result:** Car spins clockwise in place

## 🔄 Command Processing Flow

### UART Reception (Non-blocking)
```c
// In StartDefaultTask() loop:
if(HAL_UART_Receive(&huart1, &rx_byte, 1, 10) == HAL_OK) {
  // Byte received, process it
}
```
- Timeout: 10ms
- Non-blocking to allow other task operations
- Runs at 10Hz (osDelay(100))

### Command State Machine
```
┌─────────────────┐
│  Receive Byte   │
└────────┬────────┘
         ▼
┌─────────────────┐
│ Debug Print RX  │
└────────┬────────┘
         ▼
┌─────────────────┐
│  Set LED High   │
└────────┬────────┘
         ▼
┌─────────────────┐
│ Switch(rx_byte) │◄─────────────┐
└────────┬────────┘              │
         │                       │
         ├─── W/F ──► Forward    │
         ├─── S/B ──► Backward   │
         ├─── A/L ──► TurnLeft   │
         ├─── D/R ──► TurnRight  │
         ├─── Q ────► RotateLeft │
         ├─── E ────► RotateRight│
         ├─── X ────► Stop       │
         ├─── + ────► SpeedUp ───┤
         └─── - ────► SpeedDown ─┘
                ▼
         ┌─────────────────┐
         │  Set LED Low    │
         └─────────────────┘
```

### Speed Adjustment Feature
```c
case '+':
  if(current_speed < 900) current_speed += 100;
  printf("Speed: %d\r\n", current_speed);
  
  // Re-apply last movement with new speed
  switch(last_command) {
    case 'F': Motor_Forward(current_speed); break;
    case 'B': Motor_Backward(current_speed); break;
    case 'L': Motor_TurnLeft(current_speed); break;
    case 'R': Motor_TurnRight(current_speed); break;
    case 'Q': Motor_RotateLeft(current_speed); break;
    case 'E': Motor_RotateRight(current_speed); break;
  }
  break;
```

**Key Feature:** Speed changes apply immediately without resending movement command!

## 📡 ESP32 WiFi Controller

### ESP32 Code Architecture

#### Main Components
```cpp
// WiFi Access Point
const char* ap_ssid = "STM32_Car";
const char* ap_password = "12345678";
WebServer server(80);

// UART to STM32
#define STM32_SERIAL Serial2
#define STM32_TX_PIN 17
#define STM32_RX_PIN 16
#define STM32_BAUD 9600
```

#### Web Server Handlers
```cpp
void handleRoot() {
  // Serve HTML page with control buttons
  server.send_P(200, "text/html", htmlPage);
}

void handleCmd() {
  // Process button clicks
  String cmdStr = server.arg("v");
  char cmd = cmdStr.charAt(0);
  
  // Validate and send to STM32
  if(valid_command(cmd)) {
    STM32_SERIAL.write(cmd);
    server.send(200, "text/plain", "OK");
  }
}
```

#### HTML Interface
```html
<!DOCTYPE html>
<html>
<head>
  <style>
    button { width:80px; height:80px; margin:5px; }
    .stop { background:#ff4757; color:#fff; }
  </style>
</head>
<body>
  <h1>STM32 Car</h1>
  <button onclick="c('F')">FWD</button>
  <button onclick="c('L')">LEFT</button>
  <button onclick="c('S')">STOP</button>
  <button onclick="c('R')">RIGHT</button>
  
  <script>
    function c(x) {
      fetch('/c?v='+x).then(r=>r.text());
    }
  </script>
</body>
</html>
```

### Communication Protocol

**Format:** Single ASCII character per command
```
Command Byte → ESP32 → UART TX (GPIO17) → STM32 RX (PA10) → Processing
```

**Command Set:**
| Byte | ASCII | Hex  | Action |
|------|-------|------|--------|
| W/F  | 87/70 | 0x57/0x46 | Forward |
| S/B  | 83/66 | 0x53/0x42 | Backward |
| A/L  | 65/76 | 0x41/0x4C | Left |
| D/R  | 68/82 | 0x44/0x52 | Right |
| Q    | 81    | 0x51 | Rotate Left |
| E    | 69    | 0x45 | Rotate Right |
| X    | 88    | 0x58 | Stop |
| +    | 43    | 0x2B | Speed Up |
| -    | 45    | 0x2D | Speed Down |

## 🧵 FreeRTOS Task Details

### Task 1: defaultTask
```c
Stack Size: 128 words (512 bytes)
Priority: osPriorityNormal
Period: 100ms (10Hz)

Responsibilities:
- UART byte reception
- Command parsing
- Motor control execution
- LED status updates
- Speed management
```

**Stack Usage Analysis:**
- Local variables: ~20 bytes
- Function call stack: ~100 bytes
- Printf buffers: ~200 bytes
- Safety margin: ~190 bytes
- **Total: 512 bytes (adequate)**

### Task 2: GUI_Task (TouchGFX)
```c
Stack Size: 8192 words (32KB)
Priority: osPriorityNormal
Period: Variable (frame-based)

Responsibilities:
- Display framebuffer updates
- Touch event processing
- GUI animations
- Screen rendering
```

**Stack Usage Analysis:**
- TouchGFX framework: ~15KB
- Display buffers: ~10KB
- Local variables: ~2KB
- Safety margin: ~5KB
- **Total: 32KB (required by TouchGFX)**

### Memory Map
```
Flash (2MB):
├── Application Code: ~200KB
├── HAL Libraries: ~150KB
├── FreeRTOS: ~50KB
├── TouchGFX: ~300KB
└── Available: ~1.3MB

RAM (256KB):
├── .data + .bss: ~30KB
├── Heap (FreeRTOS): ~15KB
├── Stack (main): ~4KB
├── Task stacks: ~33KB
├── TouchGFX buffers: ~150KB
└── Available: ~24KB
```

## 🔍 Debugging Features

### Printf Debug Output
```c
// Startup messages
printf("USV - FIESC 3331A 2025-2026\n\r");
printf("Initializing motors...\r\n");
printf("Motor A PWM started on PB4\r\n");

// Command feedback
printf("RX: '%c' (0x%02X)\r\n", rx_byte, rx_byte);
printf("Motor_A speed=%d (PWM=%d)\r\n", speed, pwm_value);
printf("Speed: %d\r\n", current_speed);
```

### LED Indicators
```c
// Green LED (LD3) - Task heartbeat
HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);  // Every 100ms

// Red LED (LD4) - Command received
HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);    // On RX
HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);  // After processing
```

### Emergency Motor Stop
```c
// In main(), before Motor_Init()
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);  // MOTOR_A_DIR
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);  // MOTOR_B_DIR
HAL_Delay(50);  // Wait for motors to stop
```

## ⚡ Performance Characteristics

### Timing Analysis
```
Command latency breakdown:
├── WiFi transmission: ~10-50ms
├── ESP32 processing: ~1-5ms
├── UART transfer: ~1ms (1 byte @ 9600 baud)
├── STM32 reception: <1ms
├── Command processing: <1ms
├── PWM update: <0.1ms
└── Total: ~15-60ms (depends on WiFi)
```

### Throughput
```
Max command rate: ~10 Hz (limited by task delay)
UART bandwidth: 960 bytes/sec theoretical
Actual usage: ~10 bytes/sec (commands + debug)
WiFi bandwidth: Sufficient for control (HTTP overhead ~500 bytes/request)
```

### CPU Utilization
```
STM32F429 @ 180MHz:
├── defaultTask: ~5% CPU
├── GUI_Task: ~30% CPU (during rendering)
├── HAL drivers: ~5% CPU
└── Idle task: ~60% CPU
```

## 🔧 Configuration Parameters

### Adjustable Constants

#### In `main.h`:
```c
#define MOTOR_MAX_SPEED 999    // Max PWM duty cycle
#define MOTOR_MIN_SPEED 0      // Min PWM duty cycle
```

#### In `main.c`:
```c
uint16_t current_speed = 500;  // Default speed (50%)
uint16_t speed_step = 100;     // Speed increment/decrement

// TIM3 prescaler calculation
Prescaler = (SystemCoreClock / 2 / desired_freq) - 1
```

#### In ESP32 code:
```cpp
const char* ap_ssid = "STM32_Car";
const char* ap_password = "12345678";
#define STM32_BAUD 9600
```

### Customization Examples

**Change PWM frequency to 10kHz:**
```c
// In TIM3 initialization
htim3.Init.Prescaler = 8;      // 90MHz / 9 = 10MHz
htim3.Init.Period = 999;       // 10MHz / 1000 = 10kHz
```

**Add acceleration ramping:**
```c
void Motor_SetSpeed_Smooth(char motor, int16_t target_speed) {
  static int16_t current_speed_A = 0;
  static int16_t current_speed_B = 0;
  
  int16_t* current = (motor == 'A') ? &current_speed_A : &current_speed_B;
  
  // Ramp up/down in steps
  while(*current != target_speed) {
    if(*current < target_speed) *current += 10;
    else *current -= 10;
    
    Motor_SetSpeed(motor, *current);
    HAL_Delay(10);
  }
}
```

## 📚 Code Comments Convention

```c
/**
  * @brief  Function description (one line summary)
  * @param  param1: Description of parameter 1
  * @param  param2: Description of parameter 2
  * @retval Return value description
  * @note   Additional notes if needed
  */
void Example_Function(uint16_t param1, char param2);
```

---

**Next Steps:** Refer to [README.md](README.md) for usage instructions and [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md) for assembly details.
