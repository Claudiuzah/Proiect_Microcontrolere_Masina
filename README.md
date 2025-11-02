# STM32F429I Micromouse Car Project

**Course:** Microcontrollers - USV FIESC 3331A 2025-2026  
**Platform:** STM32F429I-DISCOVERY Board  
**Motor Driver:** L9100S Dual H-Bridge  
**Wireless Control:** ESP32 WiFi Module

## 📋 Project Overview

This project implements a wireless-controlled micromouse car using the STM32F429I Discovery board with TouchGFX GUI. The car can be controlled via WiFi through a web interface or directly via UART using keyboard commands.

### Key Features

- ✅ PWM-based motor speed control (0-999 speed levels)
- ✅ WiFi web interface control via ESP32
- ✅ UART keyboard control (WASD keys)
- ✅ Real-time speed adjustment while moving
- ✅ Multiple movement modes (forward, backward, turn, rotate)
- ✅ Visual LED feedback indicators
- ✅ FreeRTOS-based architecture
- ✅ TouchGFX GUI integration

## 🔧 Hardware Requirements

### Main Components
- **STM32F429I-DISCOVERY** board
- **L9100S** dual DC motor driver module
- **ESP32** development board (WiFi control)
- **2x DC motors** (for left and right wheels)
- **Battery pack** (recommended: 2x 18650 Li-ion 7.4V or 6x AA 9V)
- **Jumper wires** and breadboard

### Pin Connections

#### Motor Driver (L9100S) to STM32
| L9100S Pin | STM32 Pin | Function | Description |
|------------|-----------|----------|-------------|
| IA1 | PB4 | TIM3_CH1 | Motor A PWM (speed) |
| IA2 | PC10 | GPIO | Motor A direction |
| IB1 | PA7 | TIM3_CH2 | Motor B PWM (speed) |
| IB2 | PC11 | GPIO | Motor B direction |
| VCC | 7.4V | Power | Motor power supply |
| GND | GND | Ground | Common ground |

#### ESP32 to STM32 UART
| ESP32 Pin | STM32 Pin | Function |
|-----------|-----------|----------|
| GPIO17 (TX2) | PA10 (USART1_RX) | UART transmit |
| GPIO16 (RX2) | PA9 (USART1_TX) | UART receive |
| GND | GND | Common ground |

#### User Interface
| STM32 Pin | Function | Description |
|-----------|----------|-------------|
| PG13 | LD3 (Green LED) | Task running indicator |
| PG14 | LD4 (Red LED) | Command received indicator |
| PA0 | USER Button | User input (optional) |

### Power Connections
- **STM32**: Powered via USB (5V)
- **ESP32**: Powered via USB (5V)
- **Motors**: External battery (recommended 7.4V-9V)
- **⚠️ Important**: Connect all grounds together (STM32 GND - ESP32 GND - Battery GND)

## 📦 Software Requirements

### Development Tools
- **STM32CubeIDE** or **IAR EWARM** / **Keil MDK-ARM**
- **STM32CubeMX** (for peripheral configuration)
- **TouchGFX Designer** (optional, for GUI editing)
- **Arduino IDE** (for ESP32 programming)

### Dependencies
- **STM32 HAL Library**
- **FreeRTOS** (included in project)
- **TouchGFX Framework** (included in project)
- **ESP32 Arduino Core**
- **ESP32 WiFi Library**
- **ESP32 WebServer Library**

## 🚀 Getting Started

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/STM32-Micromouse-Car.git
cd STM32-Micromouse-Car
```

### 2. STM32 Setup

#### Open Project
1. Open STM32CubeIDE
2. Import existing project: `Proiect_uC/`
3. Build the project (Project → Build All)

#### Flash to Board
1. Connect STM32F429I-DISCOVERY via USB
2. Click Run → Debug (or press F11)
3. The program will flash and start automatically

### 3. ESP32 Setup

#### Install ESP32 Board Support
1. Open Arduino IDE
2. Go to File → Preferences
3. Add to "Additional Board Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to Tools → Board → Boards Manager
5. Search for "ESP32" and install

#### Upload Code
1. Open `ESP32_WiFi_Minimal/ESP32_WiFi_Minimal.ino`
2. Select board: Tools → Board → ESP32 Dev Module
3. Select correct COM port
4. Click Upload

### 4. Hardware Assembly
1. **Connect motors** to L9100S outputs (MA+, MA-, MB+, MB-)
2. **Wire L9100S to STM32** according to pin table above
3. **Wire ESP32 to STM32** UART (TX→RX, RX→TX, GND→GND)
4. **Connect battery** to L9100S VCC and GND
5. **⚠️ Connect all grounds together** (critical!)
6. Add **10kΩ pull-down resistors** on motor control pins (optional, prevents startup twitch)

## 🎮 Usage

### WiFi Control (ESP32)

1. **Power on** the ESP32 and STM32
2. **Connect to WiFi**:
   - SSID: `STM32_Car`
   - Password: `12345678`
3. **Open browser** and navigate to: `http://192.168.4.1`
4. **Use web interface** to control the car:
   - FWD - Move forward
   - BACK - Move backward
   - LEFT - Turn left
   - RIGHT - Turn right
   - ROT L - Rotate left (spin in place)
   - ROT R - Rotate right (spin in place)
   - STOP - Stop all motors
   - + / - - Increase/decrease speed

### UART Keyboard Control

1. **Disconnect ESP32** from USART1 (or remove WiFi control)
2. **Connect TeraTerm** (or similar) to STM32 COM port:
   - Baud rate: **9600**
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
3. **Type commands**:

| Key | Action | Description |
|-----|--------|-------------|
| W/F | Forward | Move forward |
| S/B | Backward | Move backward |
| A/L | Turn Left | Arc turn left |
| D/R | Turn Right | Arc turn right |
| Q | Rotate Left | Spin counter-clockwise |
| E | Rotate Right | Spin clockwise |
| X | Stop | Emergency stop |
| + | Speed Up | Increase speed by 100 |
| - | Speed Down | Decrease speed by 100 |

**Note:** Speed changes apply immediately while moving!

## 🔍 Project Structure

```
Proiect_uC_Masina/
├── README.md                          # This file
├── HARDWARE_GUIDE.md                  # Detailed hardware assembly
├── SOFTWARE_GUIDE.md                  # Software architecture details
├── changelog.txt                      # Version history
│
├── Proiect_uC/                        # STM32 main project
│   ├── Core/
│   │   ├── Inc/
│   │   │   ├── main.h                 # Pin definitions & prototypes
│   │   │   ├── stm32f4xx_it.h        # Interrupt handlers
│   │   │   └── FreeRTOSConfig.h      # RTOS configuration
│   │   └── Src/
│   │       ├── main.c                 # Main application & motor control
│   │       ├── freertos.c             # FreeRTOS tasks
│   │       ├── stm32f4xx_it.c        # Interrupt implementations
│   │       └── stm32f4xx_hal_msp.c   # HAL MSP (peripheral init)
│   │
│   ├── Drivers/                       # STM32 HAL drivers
│   ├── Middlewares/                   # FreeRTOS & TouchGFX
│   ├── TouchGFX/                      # GUI application
│   ├── gcc/                           # GCC build files
│   ├── MDK-ARM/                       # Keil build files
│   └── EWARM/                         # IAR build files
│
└── ESP32_WiFi_Minimal/
    └── ESP32_WiFi_Minimal.ino         # ESP32 WiFi controller
```

## 🧩 Motor Control Algorithm

### PWM Speed Control
- **Timer:** TIM3 running at 20kHz (Prescaler=89, ARR=999)
- **Speed range:** 0-999 (0% to 100% duty cycle)
- **Default speed:** 500 (50%)
- **Speed steps:** 100 (adjustable via +/- commands)

### L9100S Control Logic

#### Forward Motion
- IA1 = PWM (variable duty cycle)
- IA2 = LOW (0V)
- Result: Motor spins forward proportional to PWM

#### Backward Motion
- IA1 = PWM (inverted: 999 - speed)
- IA2 = HIGH (3.3V)
- Result: Motor spins backward proportional to PWM

#### Stop
- IA1 = 0 (no PWM)
- IA2 = LOW (0V)
- Result: Motor brake

### Movement Functions

```c
Motor_Forward(speed)      // Both motors forward
Motor_Backward(speed)     // Both motors backward
Motor_TurnLeft(speed)     // Left motor 50%, right motor 100%
Motor_TurnRight(speed)    // Left motor 100%, right motor 50%
Motor_RotateLeft(speed)   // Left backward, right forward
Motor_RotateRight(speed)  // Left forward, right backward
Motor_Stop()              // Both motors stop
```

## 🛠️ Configuration

### Changing WiFi Credentials (ESP32)
Edit `ESP32_WiFi_Minimal.ino`:
```cpp
const char* ap_ssid = "STM32_Car";        // Change SSID
const char* ap_password = "12345678";     // Change password (min 8 chars)
```

### Adjusting Motor Speed Range
Edit `Core/Inc/main.h`:
```c
#define MOTOR_MAX_SPEED 999  // Maximum PWM value
#define MOTOR_MIN_SPEED 0    // Minimum PWM value
```

### Changing Default Speed
Edit `Core/Src/main.c` in `StartDefaultTask()`:
```c
uint16_t current_speed = 500;  // Change default (0-999)
```

### Adjusting Speed Steps
Modify `+` and `-` cases in `main.c`:
```c
case '+':
  if(current_speed < 900) current_speed += 100;  // Change 100 to desired step
  break;
```

## 📊 System Architecture

### FreeRTOS Tasks
1. **defaultTask** (128 words stack, Normal priority)
   - UART command processing
   - Motor control execution
   - LED status indicators
   - Runs at 10Hz (100ms delay)

2. **GUI_Task** (8192 words stack, Normal priority)
   - TouchGFX interface updates
   - Display rendering
   - Touch event handling

### Communication Protocols
- **USART1:** 9600 baud, 8N1 (ESP32 communication)
- **USART2:** 115200 baud, 8N1 (Debug output - optional)
- **WiFi:** 802.11 b/g/n, AP mode, HTTP web server

### Interrupt Priorities (FreeRTOS)
- **DMA2D:** Priority 5
- **LTDC:** Priority 5
- **USART1:** Priority 5
- **TIM6:** Priority 15 (FreeRTOS tick)

## 🐛 Troubleshooting

### Motors spin backward on reset
**Cause:** GPIO pins in undefined state during boot  
**Solution:** 
- Add 10kΩ pull-down resistors to IA1, IA2, IB1, IB2
- Code includes emergency motor stop at startup

### Motors spin slowly
**Cause:** Insufficient battery voltage/current  
**Solution:**
- Use 2x 18650 Li-ion (7.4V) instead of AAA batteries
- Or use 6x AA batteries (9V)
- Check battery charge level

### ESP32 commands not received
**Cause:** Missing common ground or TeraTerm blocking port  
**Solution:**
- Connect GND between ESP32 and STM32
- Disconnect TeraTerm when using ESP32 control
- Check TX→RX, RX→TX wiring (must be crossed)

### No WiFi connection
**Cause:** ESP32 not programmed or wrong credentials  
**Solution:**
- Re-upload ESP32 code
- Check serial monitor for "STM32_Car" SSID confirmation
- Verify password is at least 8 characters

### Motors don't respond to commands
**Cause:** Battery too weak or wiring error  
**Solution:**
- Check battery voltage (should be >6V under load)
- Verify L9100S connections match pin table
- Test with higher PWM values (speed 700-900)

### No debug output in TeraTerm
**Cause:** Wrong COM port or baud rate  
**Solution:**
- Check Device Manager for correct COM port
- Set baud rate to 9600 (USART1) or 115200 (USART2)
- Verify USB cable supports data transfer

## 🔄 Future Improvements

- [ ] Add ultrasonic sensors for obstacle detection
- [ ] Implement maze-solving algorithm
- [ ] Add battery voltage monitoring
- [ ] Implement PID speed control
- [ ] Add encoder feedback for precise movements
- [ ] Create smartphone app (Android/iOS)
- [ ] Add Bluetooth control as alternative to WiFi
- [ ] Implement autonomous navigation mode
- [ ] Add line-following capability
- [ ] Create data logging to SD card

## 📝 License

This project is developed for educational purposes as part of the Microcontrollers course at USV FIESC.

## 👥 Contributors

- **Student Name** - Main developer
- **Course:** Microcontrollers (FIESC 3331A)
- **Year:** 2025-2026
- **Institution:** USV (Universitatea Stefan cel Mare din Suceava)

## 📧 Contact

For questions or issues, please open an issue on GitHub or contact via university email.

## 🙏 Acknowledgments

- STMicroelectronics for STM32CubeIDE and HAL libraries
- TouchGFX team for GUI framework
- Espressif Systems for ESP32 Arduino core
- Course instructors and lab assistants

---

**⚠️ Safety Notes:**
- Never connect motor battery directly to STM32 (use L9100S as intermediary)
- Ensure proper heatsinking for L9100S if running high currents
- Use appropriate battery protection (especially for Li-ion/LiPo)
- Keep motor driver away from microcontroller to avoid EMI
- Add decoupling capacitors near motor driver (100nF + 10µF)

**📌 Version:** 1.0  
**📅 Last Updated:** November 2025
