# ADF4351 Arduino Library & KiCad Hardware Design
| KiCad Hardware PCB Design | Software Output / RF Spectrum |
| :---: | :---: |
| <img width="1578" height="930" alt="Front" src="https://github.com/user-attachments/assets/2eac5e87-a385-4080-a6b1-3c701cb6a450" />| https://github.com/user-attachments/assets/7dd1f7be-7177-40c9-8f7e-6cccfdf0b84d |

A complete open-source RF signal generation suite featuring a **lightweight C++ driver** and a **custom open-hardware PCB design in KiCad** for the Analog Devices ADF4351 wideband frequency synthesizer (35 MHz – 4.4 GHz).

### What This Project Provides
1. Software (Arduino Driver & Math Engine)
    - Register Calculation Engine: Automatically solves the complex Phase-Locked Loop (PLL) math to determine exact 32-bit values for hardware registers $R_0$ through $R_5$ across both Fractional-N and Integer-N modes.
    - Automatic Scaling: Handles RF output dividers (/1 to /64), dual-modulus prescalers (4/5 or 8/9), and GCD fraction reduction behind the scenes.
    - Hardware-Agnostic C++: Contains no hardcoded hardware SPI pins or board-specific hooks. Works seamlessly across all Arduino architectures, including AVR, ESP32, STM32, RP2040, Teensy, and SAMD, using standard SPI.h.
2. Hardware (KiCad Design Files)
    - Open Hardware Files: Complete schematics (.kicad_sch), 4-layer impedance-controlled PCB layouts (.kicad_pcb), and project files created in KiCad.
    - RF Output Network: Designed with proper high-frequency decoupling, low-noise power regulation, optimized loop filtering, and 50 Ω matched RF output traces with SMA connectivity for clean signal generation across the 35 MHz – 4.4 GHz spectrum.

## Table of Contents
- Features
- Installation
- Quick Start
- API Reference & Keyword Usage
  - Core Datatypes & Classes
  - Core Functions
  - Bulk Configuration
  - Register 5 Setters (Lock Detect & Initialization)
  - Register 4 Setters (RF Output & Power Controls)
  - Register 3 Setters (Clock Dividers & Modes)
  - Register 2 Setters (PLL & Charge Pump Controls)
  - Register 1 & 0 Setters (Prescaler & Phase Settings)
- Hardware Overview
- Hardware Files
- License

## Features
- **Platform Agnostic:** Pure software calculation engine with zero hardware or architecture dependencies.
- **Dynamic PLL Solver:** Automates INT, FRAC, and MOD calculations with automated Euclidean GCD fraction reduction.
- **VCO & Divider Scaling:** Dynamically selects RF output dividers (/1 to /64) and prescalers (4/5 vs. 8/9).
- **Integrated Bitmask Safety:** All parameter setters perform bitwise masking to prevent register corruption.
- **KiCad Open Hardware Included:** Includes full schematic, PCB layout, and gerber files in the `hardware/` directory.

## Installation
### Via Arduino Library Manager
1. Open the Arduino IDE.
2. Go to Sketch → Include Library → Manage Libraries...
3. Search for ADF4351 and click Install.

### Manual Installation
1. Download the latest release .zip from GitHub.
2. In the Arduino IDE, go to Sketch → Include Library → Add .ZIP Library...
3. Select the downloaded .zip file.

## Quick Start
~~~
#include <Arduino.h>
#include "ADF4351.h"

const int CS_PIN = 10;
ADF4351 adf;

// Helper function to send 32-bit register words over hardware SPI
void writeRegister(uint32_t regData) {
    digitalWrite(CS_PIN, LOW);
    SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    SPI.transfer32(regValue);
    SPI.endTransaction();
    digitalWrite(CS_PIN, HIGH);
    delayMicroseconds(1);
}

void setup() {
    Serial.begin(115200);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    SPI.begin();

    // 1. Configure synthesizer parameters (optional step; default is 25MHz ref clock)
    adf.setRefFreqHz(25000000);  // 25 MHz Reference
    adf.setOutputPower(3);       // +5 dBm Output Power

    // 2. Compute registers for target frequency (e.g., 433.92 MHz)
    uint32_t regs[6];
    if (adf.calculateRegisters(433920000ULL, regs)) {
        // Write registers R5 down to R0 in sequence (ADF4351 hardware requirement)
        for (int i = 5; i >= 0; i--) {
            writeRegister(regs[i]);
        }
        Serial.println(F("ADF4351 programmed successfully to 433.92 MHz!"));
    } else {
        Serial.println(F("Frequency target out of bounds or parameter error!"));
    }
}

void loop() {
    // Idle
}
~~~

## API Reference & Keyword Usage
### Core Datatypes & Classes
`ADF4351`

The primary class representing the ADF4351 synthesizer instance.
~~~
ADF4351 adf; // Instantiate with default configuration (25MHz TCXO/OCXO ref)
~~~
____
`Config`

A configuration structure containing all chip settings.
~~~
ADF4351::Config myConfig;
myConfig.refFreqHz = 10000000; // Set reference clock to 10 MHz
myConfig.outputPower = 2;      // Set power to +2 dBm
~~~

### Core Functions
`calculateRegisters(freqHz, outRegisters)`

Computes the 6 required 32-bit register values for a given frequency in Hertz.
- Parameters:
  - `freqHz (uint64_t)`: Target output frequency in Hz (35,000,000 to 4,400,000,000 Hz).
  - `outRegisters (uint32_t[6])`: Array to hold the resulting register words R0 through R5.
- Returns: bool – true if calculation succeeded; false if frequency is out of range or invalid.
~~~
uint32_t registers[6];
bool success = adf.calculateRegisters(1000000000ULL, registers); // 1.0 GHz
~~~

### Bulk Configuration
`setConfig(config)`

Overwrites the entire configuration struct in one operation.
~~~
ADF4351::Config cfg;
cfg.refFreqHz = 25000000;
cfg.outputPower = 3;
cfg.rfOutputEnable = 1;

adf.setConfig(cfg);
~~~
____
`getConfig()`

Retrieves the currently loaded configuration struct.
~~~
ADF4351::Config activeCfg = adf.getConfig();
Serial.print("Current Ref Freq: ");
Serial.println(activeCfg.refFreqHz);
~~~

### Individual Configuration Setting
#### Register 5 Setters
`setRefFreqHz(val)`

Sets the reference input frequency in Hz.
~~~
adf.setRefFreqHz(25000000); // 25 MHz Reference
~~~
___
`setLdPinMode(val)`

Configures the Lock Detect (LD) output pin mode (Bits [23:22], Mask: 0x03).
- 0: Low
- 1: Digital Lock Detect (Default)
- 2: Low
- 3: High
~~~
adf.setLdPinMode(1); // Digital Lock Detect
~~~

#### Register 4 Setters
`setFeedbackSelect(val)`

Selects VCO feedback signal source to N-counter (Bit [23], Mask: 0x01).
- 0: Divided (VCO output through divider)
- 1: Fundamental (VCO output directly; Default)
~~~
adf.setFeedbackSelect(1);
~~~
___
`setVcoPowerdown(val)`

Powers down the internal VCO (Bit [11], Mask: 0x01).
- 0: VCO Active
- 1: VCO Powered Down
~~~
adf.setVcoPowerdown(0);
~~~
___
`setMtld(val)`

Mute Till Lock Detect (Bit [10], Mask: 0x01).
- 0: Mute Disabled
- 1: Mute Enabled (RF output muted until PLL locks)
~~~
adf.setMtld(1);
~~~
___
`setAuxOutputSelect(val)`

Selects auxiliary RF output signal path (Bit [9], Mask: 0x01).
- 0: Divided Output
- 1: Fundamental Output
~~~
adf.setAuxOutputSelect(0);
~~~
___
`setAuxOutputEnable(val)`

Enables or disables auxiliary RF output pin (Bit [8], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setAuxOutputEnable(0);
~~~
___
`setAuxOutputPower(val)`

Configures auxiliary RF output power level (Bits [7:6], Mask: 0x03).
- 0: -4 dBm
- 1: -1 dBm
- 2: +2 dBm
- 3: +5 dBm
~~~
adf.setAuxOutputPower(0); // -4 dBm
~~~
___
`setRfOutputEnable(val)`

Main RF output power toggle (Bit [5], Mask: 0x01).
- 0: Output Disabled
- 1: Output Enabled
~~~
adf.setRfOutputEnable(1);
~~~
___
`setOutputPower(val)`

Sets main RF output power level (Bits [4:3], Mask: 0x03).
- 0: -4 dBm
- 1: -1 dBm
- 2: +2 dBm
- 3: +5 dBm
~~~
adf.setOutputPower(3); // +5 dBm maximum power
~~~

#### Register 3 Setters
`setBandSelectMode(val)`

Configures Band Select Clock logic mode (Bit [23], Mask: 0x01).
- 0: Low / Standard PFD frequency
- 1: High / Fast PFD frequency
~~~
adf.setBandSelectMode(0);
~~~
___
`setAbp(val)`

Anti-Backlash Pulse Width (Bit [22], Mask: 0x01).
- 0: 6 ns (Recommended for Fractional-N operation)
- 1: 3 ns (Recommended for Integer-N operation)
~~~
adf.setAbp(0);
~~~
___
`setChargeCancel(val)`

Charge pump cancellation control (Bit [21], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setChargeCancel(0);
~~~
___
`setCsr(val)`

Cycle Slip Reduction (Bit [18], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setCsr(0);
~~~
___
`setClkDivMode(val)`

Clock Divider Mode (Bits [16:15], Mask: 0x03).
- 0: Clock Divider Off
- 1: Fast Lock Enable
- 2: Resynchronization Enable
~~~
adf.setClkDivMode(0);
~~~
___
`setClkDivValue(val)`

12-bit clock divider value for fast-lock / resync (Bits [14:3], Mask: 0x0FFF).

Range: 0 to 4095.
~~~
adf.setClkDivValue(150);
~~~

#### Register 2 Setters
`setNoiseMode(val)`

Low Noise vs. Low Spur Mode configuration (Bits [30:29], Mask: 0x03).
- 0: Low Noise Mode
- 3: Low Spur Mode
~~~
adf.setNoiseMode(0); // Low noise mode
~~~
___
`setMuxout(val)`

Configures the multiplexer output pin function (Bits [28:26], Mask: 0x07).
- 0: Three-State Output
- 1: DVDD
- 2: DGND
- 3: R-Divider Output
- 4: N-Divider Output
- 5: Analog Lock Detect
- 6: Digital Lock Detect
~~~
adf.setMuxout(0);
~~~
___
`setReferenceDoubler(val)`

Enables input reference frequency multiplier (Bit [25], Mask: 0x01).
- 0: Disabled
- 1: Enabled (Doubles PFD frequency)
~~~
adf.setReferenceDoubler(0);
~~~
___
`setRdiv2(val)`

Reference Divide-by-2 toggle (Bit [24], Mask: 0x01).
- 0: Disabled
- 1: Divide reference clock by 2 before R-divider
~~~
adf.setRdiv2(0);
~~~
___
`setRDivider(val)`

10-bit Reference R-Divider value (Bits [23:14], Mask: 0x03FF). 

Range: 1 to 1023.
~~~
adf.setRDivider(1);
~~~
___
`setDoubleBuff(val)`

Double buffering control for Register 4 (Bit [13], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setDoubleBuff(0);
~~~
___
`setChargePumpCurr(val)`

Configures charge pump current setting (Bits [12:9], Mask: 0x0F).

Range: 0 (0.31 mA) to 15 (5.00 mA). Default 7 = 2.50 mA.
~~~
adf.setChargePumpCurr(7); // 2.50 mA
~~~
___
`setLdf(val)`

Lock Detect Functionality selection (Bit [8], Mask: 0x01).
- 0: Fractional-N Mode Lock Detect
- 1: Integer-N Mode Lock Detect
~~~
adf.setLdf(0);
~~~
___
`setLdp(val)`

Lock Detect Precision pulse threshold (Bit [7], Mask: 0x01).
- 0: 10 ns (40 consecutive cycles)
- 1: 6 ns (40 consecutive cycles)
~~~
adf.setLdp(0);
~~~
___
`setPdPolarity(val)`

Phase Detector Polarity (Bit [6], Mask: 0x01).
- 0: Negative (Non-inverting loop filter)
- 1: Positive (Inverting loop filter / active filter)
~~~
adf.setPdPolarity(1);
~~~
___
`setPowerDown(val)`

Software chip power-down mode (Bit [5], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setPowerDown(0);
~~~
___
`setCpThreeState(val)`

Charge Pump Three-State control (Bit [4], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setCpThreeState(0);
~~~
___
`setCounterReset(val)`

Resets internal N and R counters (Bit [3], Mask: 0x01).
- 0: Disabled
- 1: Enabled
~~~
adf.setCounterReset(0);
~~~

#### Register 1 & 0 Setters
`setAutoPrescaler(enable)`

Enables automated dynamic prescaler selection (4/5 vs. 8/9).

Can be _true_ or _false_.
~~~
adf.setAutoPrescaler(false);
~~~
___
`setPrescaler(val)`

Manually overrides the dual-modulus prescaler setting (Bit [27], Mask: 0x01). Automatically disables autoPrescaler.
- 0: 4/5 Prescaler
- 1: 8/9 Prescaler
~~~
adf.setPrescaler(1); // Manual 8/9 prescaler
~~~
___
`setPhaseAdjust(val)`

Phase adjustment control toggle (Bit [28], Mask: 0x01).
- 0: OFF
- 1: ON
~~~
adf.setPhaseAdjust(0);
~~~
___
`setPhaseValue(val)`

12-bit fractional phase word (Bits [26:15], Mask: 0x0FFF). Range: 0 to 4095.
~~~
adf.setPhaseValue(1);
~~~

## Hardware Overview
The ADF4351 features an integrated Voltage Controlled Oscillator (VCO) operating natively between 2200 MHz and 4400 MHz. Lower output frequencies (35 MHz to 2200 MHz) are generated using internal divide-by-1, 2, 4, 8, 16, 32, or 64 output dividers.


                                      
## Hardware Files
Complete open-source hardware design files created in KiCad are available in the hardware/ directory:
- ADF4351.kicad_sch: Complete schematic diagram with power filtering and RF matching networks.
- ADF4351.kicad_pcb: 2-layer controlled impedance PCB layout.
- ADF4351.kicad_pro: KiCad project workspace file.

## License
This library is released under the MIT License. Free to use, modify, and distribute in both open-source and commercial applications.
