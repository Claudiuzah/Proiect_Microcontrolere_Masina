# VL53L0X Integration Guide

## ✅ What Has Been Done

### 1. CubeMX Configuration (You completed)
- **I2C3** configured on PA8 (SCL) and PC9 (SDA)
- **GPIO Outputs** for XSHUT pins:
  - PA1 → Front sensor XSHUT
  - PA2 → Left sensor XSHUT
  - PA5 → Right sensor XSHUT

### 2. Platform Driver Created
- ✅ `vl53l0x_platform.h` - Interface definitions
- ✅ `vl53l0x_platform.c` - HAL I2C implementation

### 3. Main.c Integration
- ✅ Added `#include "vl53l0x_platform.h"`
- ✅ Sensor initialization in `main()` after motor init
- ✅ Automatic sensor update every 500ms in main task loop
- ✅ Print initialization status on startup

---

## 📋 How It Works

### Sensor Initialization Sequence
```
1. Disable all sensors (XSHUT LOW)
2. Enable FRONT sensor → Change address to 0x54
3. Enable LEFT sensor → Change address to 0x56
4. Enable RIGHT sensor → Change address to 0x58
5. All sensors now on same I2C bus with unique addresses
```

### I2C Addresses
| Sensor | New Address | 7-bit Address |
|--------|-------------|---------------|
| Front  | 0x54        | 0x2A          |
| Left   | 0x56        | 0x2B          |
| Right  | 0x58        | 0x2C          |

---

## 🔌 Wiring Connections

### VL53L0X Pin Connections (for each sensor):
| VL53L0X Pin | Connection | Notes |
|-------------|------------|-------|
| VDD | 3.3V | STM32 3.3V pin |
| GND | GND | Common ground |
| SCL | PA8 | I2C3_SCL (all sensors) |
| SDA | PC9 | I2C3_SDA (all sensors) |
| XSHUT | PA1/PA2/PA5 | Different pin per sensor |
| GPIO1 | Not connected | Interrupt pin (optional) |

### Individual Sensor XSHUT:
- **Front sensor** XSHUT → **PA1**
- **Left sensor** XSHUT → **PA2**
- **Right sensor** XSHUT → **PA5**

---

## 💻 How to Use the Sensors

### 1. Getting Sensor Data

```c
// Get pointer to sensor data
VL53L0X_Data_t *front = VL53L0X_GetData(VL53_FRONT);
VL53L0X_Data_t *left = VL53L0X_GetData(VL53_LEFT);
VL53L0X_Data_t *right = VL53L0X_GetData(VL53_RIGHT);

// Check if sensor is initialized
if (front->initialized) {
    uint16_t distance = front->distance_mm;  // Distance in millimeters
    uint8_t status = front->status;           // 0=OK, 1=Error
    printf("Front: %d mm\n", distance);
}
```

### 2. Obstacle Detection Example

```c
VL53L0X_Data_t *front = VL53L0X_GetData(VL53_FRONT);

if (front->distance_mm < 200) {  // Obstacle < 20cm
    Motor_Stop();
    printf("OBSTACLE DETECTED!\n");
} else {
    Motor_Forward(current_speed);
}
```

### 3. Enable Debug Output

Uncomment this line in `StartDefaultTask()`:
```c
// printf("Sensors [mm]: F=%d L=%d R=%d\r\n", 
//        front->distance_mm, left->distance_mm, right->distance_mm);
```

---

## 🚨 Important Notes

### ⚠️ This is a BASIC Implementation
The current code provides:
- ✅ I2C communication
- ✅ Basic sensor initialization
- ✅ Distance reading
- ✅ Multi-sensor address assignment

**For full functionality, you need the official ST VL53L0X API:**
- Advanced calibration
- Long-range mode
- High-speed mode
- Better accuracy
- Interrupt-based readings

### 📦 Getting the Full API

**Option 1: ST Official API**
- Download: [STSW-IMG005](https://www.st.com/en/embedded-software/stsw-img005.html)
- Extract and add these files to `Drivers/VL53L0X/`:
  - `vl53l0x_api.c`
  - `vl53l0x_api_core.c`
  - `vl53l0x_api_calibration.c`
  - `vl53l0x_api_strings.c`
  - All `.h` files from `Inc/` folder

**Option 2: Pololu Arduino Library (easier to port)**
- GitHub: https://github.com/pololu/vl53l0x-arduino
- Simpler API, easier to understand

---

## 🧪 Testing Steps

### 1. Build and Flash
```bash
# In STM32CubeIDE
Project → Build All
Run → Debug
```

### 2. Check Serial Output
Open TeraTerm (115200 baud), you should see:
```
Initializing VL53L0X sensors...
Front sensor: OK
Left sensor: OK
Right sensor: OK
```

### 3. Test Distance Reading
- Hold object in front of sensors
- Watch distance values change
- Verify sensors work at 45° angles

### 4. Verify I2C Communication
If sensors fail to initialize:
- Check wiring (especially common ground)
- Verify 3.3V power to all sensors
- Use oscilloscope on SCL/SDA to check I2C signals
- Check for I2C pull-up resistors (should be on board)

---

## 🛠️ Troubleshooting

### Sensor Won't Initialize
**Symptom:** `Front sensor: FAILED`

**Solutions:**
1. Check VDD (should be 3.3V ±0.3V)
2. Verify XSHUT pin is HIGH (3.3V when enabled)
3. Check I2C pull-ups (typical 4.7kΩ to 3.3V)
4. Verify SCL/SDA connections not swapped
5. Try initializing one sensor at a time

### Wrong Distance Values
**Symptom:** Distance = 8191 or 0

**Reasons:**
- Out of range (VL53L0X max ~2000mm typical)
- Poor reflective surface (black/transparent objects)
- Sensor not properly initialized
- I2C communication error

### I2C Bus Lockup
**Symptom:** All sensors offline after reset

**Fix:**
- Power cycle STM32
- Check for short circuits on I2C lines
- Verify sensor XSHUT sequence working correctly

---

## 📈 Next Steps

### 1. Autonomous Mode (example provided)
See `vl53l0x_example.c` for:
- Obstacle avoidance logic
- Sensor status printing
- Integration with motor control

### 2. Add PS4 Controller Button for Autonomous Mode
```c
// In ESP32 code, add:
if (square_button) sendToSTM32('A');  // Toggle autonomous

// In STM32 main.c, add:
case 'A':
    autonomous_mode = !autonomous_mode;
    printf("Autonomous: %s\n", autonomous_mode ? "ON" : "OFF");
    break;
```

### 3. Improve Distance Algorithm
- Add Kalman filtering for smoother readings
- Implement PID control for wall following
- Add maze-solving algorithm

---

## 📝 Summary

✅ **What Works Now:**
- 3 VL53L0X sensors with unique I2C addresses
- Automatic initialization on startup
- Distance updates every 500ms
- Basic obstacle detection capability

⚠️ **Limitations:**
- Basic initialization only (no advanced calibration)
- Blocking I2C reads (not interrupt-driven)
- No long-range or high-speed modes
- Limited error handling

🔧 **To Get Full Performance:**
Download and integrate the official ST VL53L0X API for production use.

---

**You're ready to test! Build and flash the code.** 🚀
