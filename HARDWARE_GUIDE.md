# Hardware Assembly Guide

## 🔌 Complete Wiring Diagram

### Components Overview
```
┌─────────────────┐      ┌──────────────┐      ┌─────────────┐
│   STM32F429I    │◄────►│   ESP32      │      │   L9100S    │
│   DISCOVERY     │ UART │   WiFi       │      │ Motor Driver│
└────────┬────────┘      └──────────────┘      └──────┬──────┘
         │                                             │
         └─────────────────────────────────────────────┘
                     Motor Control Signals
                     
                                                ┌─────────────┐
                                                │  DC Motors  │
                                                │  Left/Right │
                                                └─────────────┘
```

## 📋 Bill of Materials (BOM)

### Essential Components
| Item | Quantity | Specifications | Estimated Cost |
|------|----------|----------------|----------------|
| STM32F429I-DISCOVERY | 1 | Official board | $25-30 |
| ESP32 Dev Board | 1 | ESP32-WROOM-32 | $5-8 |
| L9100S Motor Driver | 1 | Dual H-bridge module | $2-3 |
| DC Motors | 2 | 3-6V, geared | $5-10 |
| 18650 Batteries | 2 | 3.7V, 2000mAh+ | $8-12 |
| 18650 Battery Holder | 1 | 2x series holder | $2-3 |
| Jumper Wires | 20+ | Male-to-male/female | $3-5 |
| Breadboard | 1 | Half or full size | $3-5 |

### Recommended Additional Components
| Item | Quantity | Purpose | Estimated Cost |
|------|----------|---------|----------------|
| 10kΩ Resistors | 4 | Pull-down for motor pins | $0.50 |
| 100nF Capacitors | 4 | Decoupling | $1 |
| 10µF Capacitors | 2 | Power filtering | $1 |
| Wheels | 2 | Match motor shaft | $3-5 |
| Caster Wheel | 1 | Front/rear support | $2-3 |
| Chassis | 1 | Robot base platform | $5-10 |
| USB Cables | 2 | Power & programming | $5 |

**Total Estimated Cost:** $70-110 USD

## 🔧 Step-by-Step Assembly

### Step 1: Prepare the L9100S Motor Driver

#### Pin Identification
```
L9100S Module Pinout:
┌─────────────────────┐
│  VCC  GND  IA1  IA2 │  ← Input Side
│   │    │    │    │  │
│  [L9100S CHIP]      │
│   │    │    │    │  │
│  MA+  MA-  IB1  IB2 │
│   │    │    │    │  │
│  MB+  MB-           │  ← Output Side
└─────────────────────┘
```

#### Motor Connections
1. Connect **left motor** to **MA+ and MA-**
2. Connect **right motor** to **MB+ and MB-**
3. Note polarity - if motor spins backward, swap the wires

### Step 2: Wire L9100S to STM32

#### Required Connections
```
L9100S Pin    →    STM32 Pin    Function
──────────────────────────────────────────
IA1           →    PB4          Motor A PWM (TIM3_CH1)
IA2           →    PC10         Motor A Direction
IB1           →    PA7          Motor B PWM (TIM3_CH2)
IB2           →    PC11         Motor B Direction
VCC           →    Battery+     7.4V power
GND           →    STM32 GND    Common ground
```

#### Wiring Tips
- Use color coding: Red (PWM), Black (GND), Yellow (Direction)
- Keep motor wires short to reduce EMI
- Route power and signal wires separately if possible

### Step 3: Wire ESP32 to STM32

#### UART Connection
```
ESP32 Pin     →    STM32 Pin    Function
──────────────────────────────────────────
GPIO17 (TX2)  →    PA10         USART1_RX
GPIO16 (RX2)  →    PA9          USART1_TX
GND           →    GND          Common ground
```

#### Important Notes
- **TX goes to RX, RX goes to TX** (crossover connection)
- Both ESP32 and STM32 use 3.3V logic (no level shifter needed)
- Keep UART wires twisted together to reduce noise

### Step 4: Power Distribution

#### Power Architecture
```
USB 5V (STM32) ──→ STM32F429I ──→ 3.3V Logic
                                    │
                                    └──→ ESP32 (via UART)

USB 5V (ESP32) ──→ ESP32 WiFi Module

Battery 7.4V ──→ L9100S VCC ──→ DC Motors (6-8V)
         └─────→ GND ──→ Common Ground
```

#### Critical: Common Ground
**All grounds MUST be connected together:**
```
STM32 GND ──┬── ESP32 GND ──┬── Battery GND (-)
            │               │
            └───────────────┴── L9100S GND
```

### Step 5: Add Pull-down Resistors (Recommended)

#### Why Pull-downs?
During STM32 reset, GPIO pins are in high-impedance state. This can cause motors to spin unexpectedly.

#### Resistor Placement
```
STM32 Pin  →  10kΩ Resistor  →  GND
─────────────────────────────────────
PB4 (IA1)  →      10kΩ       →  GND
PC10 (IA2) →      10kΩ       →  GND
PA7 (IB1)  →      10kΩ       →  GND
PC11 (IB2) →      10kΩ       →  GND
```

Connect resistors close to the L9100S input pins.

### Step 6: Add Decoupling Capacitors (Recommended)

#### Capacitor Placement
```
Near L9100S VCC:
  - 100nF ceramic (between VCC and GND)
  - 10µF electrolytic (between VCC and GND)

Near each motor terminal:
  - 100nF ceramic (between motor+ and motor-)
```

Purpose: Reduce electrical noise from motor commutation

## 📐 Physical Assembly

### Mounting Layout Suggestion
```
Top View of Chassis:
┌──────────────────────────────────┐
│                                  │
│  [STM32F429I Board]              │
│         ↓USB                     │
│                                  │
│  [ESP32]      [L9100S]           │
│   ↓USB         Module            │
│                                  │
│  [Battery Pack]                  │
│   2x18650                        │
│                                  │
│  [Motor]              [Motor]    │
│   Left                  Right    │
└──────────────────────────────────┘
        [Caster Wheel]
```

### Mounting Tips
1. **STM32:** Use spacers to elevate board (prevents shorts)
2. **ESP32:** Mount away from motors to reduce EMI
3. **L9100S:** Ensure good airflow (can get warm)
4. **Battery:** Secure firmly (center of gravity)
5. **Motors:** Align wheels parallel for straight driving

## 🔍 Testing Procedure

### Phase 1: Power Test (No Motors)
1. Connect only STM32 to USB
2. Verify green power LED lights up
3. Connect ESP32 to USB
4. Check ESP32 power LED

### Phase 2: Communication Test
1. Wire ESP32 UART to STM32
2. Upload ESP32 code
3. Flash STM32 code
4. Check ESP32 serial monitor for "READY!"
5. Connect to WiFi "STM32_Car"

### Phase 3: Motor Driver Test (No Load)
1. Wire L9100S to STM32 (without motors)
2. Connect battery to L9100S
3. Send test commands
4. Measure voltage on MA+/MA- with multimeter
5. Should see varying voltage (0-7.4V) with PWM

### Phase 4: Full System Test
1. Connect motors to L9100S
2. Place car on blocks (wheels off ground)
3. Send forward command
4. Verify both wheels spin forward
5. Test all directions

### Phase 5: Load Test
1. Place car on ground
2. Test movement in all directions
3. Monitor battery voltage under load
4. Check for overheating on L9100S

## ⚠️ Common Wiring Mistakes

### 1. TX-TX / RX-RX Connection
**Wrong:** ESP32 TX → STM32 TX  
**Correct:** ESP32 TX → STM32 RX

### 2. Forgotten Common Ground
**Symptom:** Erratic behavior, no communication  
**Fix:** Connect all grounds together

### 3. Motor Power to STM32
**Wrong:** Battery 7.4V → STM32 VCC  
**Correct:** Battery → L9100S only

### 4. Swapped Motor Polarity
**Symptom:** Car goes backward when commanded forward  
**Fix:** Swap MA+ and MA- (or MB+ and MB-)

### 5. Missing Pull-down Resistors
**Symptom:** Motors twitch on reset  
**Fix:** Add 10kΩ resistors to IA1, IA2, IB1, IB2

## 🛡️ Safety Checklist

- [ ] Battery voltage is within L9100S range (2.5V-12V)
- [ ] All grounds are connected together
- [ ] No wires are crossed or shorted
- [ ] STM32 receives only 5V from USB (not motor voltage)
- [ ] L9100S has adequate ventilation
- [ ] Battery has proper protection circuit (especially Li-ion)
- [ ] Motor current is within L9100S rating (800mA per channel)
- [ ] Wires are secured and won't get caught in wheels
- [ ] Emergency stop method is available (press RESET or remove battery)

## 🔋 Battery Selection Guide

### Option 1: 2x 18650 Li-ion (BEST for micromouse)
- **Voltage:** 7.4V nominal (8.4V full, 6V empty)
- **Capacity:** 2000-3000mAh typical
- **Discharge:** 2-10A continuous
- **Weight:** ~90g
- **Runtime:** 1-2 hours
- **Pros:** Best power-to-weight, rechargeable
- **Cons:** Requires protection circuit, special charger

### Option 2: 6x AA Alkaline (EASIEST)
- **Voltage:** 9V nominal (drops to 6V under load)
- **Capacity:** 2000-3000mAh
- **Discharge:** 0.5-1A continuous
- **Weight:** ~140g
- **Runtime:** 30-60 minutes
- **Pros:** Easy to find, cheap, safe
- **Cons:** Not rechargeable, heavier

### Option 3: 2S LiPo (BEST performance)
- **Voltage:** 7.4V nominal (8.4V full, 6V empty)
- **Capacity:** 500-1500mAh typical
- **Discharge:** 20-30C (10-30A!)
- **Weight:** ~60-80g
- **Runtime:** 30-90 minutes
- **Pros:** Lightest, highest discharge rate
- **Cons:** Needs balance charger, safety concerns

### NOT Recommended
- ❌ 4x AAA (6V) - Too weak, voltage sag
- ❌ 9V alkaline single - Very low capacity (~500mAh)
- ❌ USB power bank - Not enough current

## 📊 Expected Performance

With proper assembly:
- **Top speed:** ~0.5-1.5 m/s (depends on motors)
- **Runtime:** 30-120 minutes (depends on battery)
- **WiFi range:** 10-30 meters (depends on environment)
- **Response time:** <100ms command to action
- **Turn radius:** ~15-30cm (depends on wheelbase)

## 🔧 Maintenance

### Weekly
- Check all wire connections for looseness
- Verify battery charge level
- Clean motor contacts if performance degrades

### Monthly
- Inspect L9100S for overheating signs
- Check wheel alignment
- Test battery capacity (runtime)

### As Needed
- Replace batteries when capacity drops
- Clean motor brushes if noisy
- Resolder any cold joints

---

**Next Steps:** After hardware assembly, proceed to [SOFTWARE_GUIDE.md](SOFTWARE_GUIDE.md) for programming instructions.
