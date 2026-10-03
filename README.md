
# ADF4351 Arduino Library

A lightweight, architecture-agnostic C++ driver and register calculation engine for the **Analog Devices ADF4351** wideband synthesized signal generator (35 MHz – 4.4 GHz).

This library computes the exact 32-bit hardware register values (\(R_0\) through \(R_5\)) for Fractional-N and Integer-N Phase-Locked Loop (PLL) operating modes without enforcing SPI pin hardware bindings. It can be used on any Arduino-compatible board (AVR, ESP32, STM32, RP2040, Teensy, SAMD) alongside your board's standard `SPI` implementation.

## Table of Contents
- Features
- Installation
- Hardware Overview
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
- Hardware Files
- License

## Features
- **Platform Agnostic:** Pure software calculation engine with zero hardware or architecture dependencies.

Dynamic PLL Solver: Automates INT, FRAC, and MOD calculations with automated Euclidean GCD fraction reduction.

VCO & Divider Scaling: Dynamically selects RF output dividers (/1 to /64) and prescalers (4/5 vs. 8/9).

Integrated Bitmask Safety: All parameter setters perform bitwise masking to prevent register corruption.

KiCad Open Hardware Included: Includes full schematic and PCB layout files in the hardware/ directory.

Installation
Via Arduino Library Manager
Open the Arduino IDE.

Go to Sketch → Include Library → Manage Libraries...

Search for ADF4351 and click Install.

Manual Installation
Download the latest release .zip from GitHub.

In the Arduino IDE, go to Sketch → Include Library → Add .ZIP Library...

Select the downloaded .zip file.

Hardware Overview
The ADF4351 features an integrated Voltage Controlled Oscillator (VCO) operating natively between 2200 MHz and 4400 MHz. Lower output frequencies (35 MHz to 2200 MHz) are generated using internal divide-by-1, 2, 4, 8, 16, 32, or 64 output dividers.

Plaintext
                  +-----------------------------------+
                  |              ADF4351              |
                  |                                   |
Ref Clock IN ---->| [R Divider] -> PFD -> Phase Det   |
                  |                        |          |
                  |                       VCO (2.2-4.4GHz)
                  |                        |          |
                  |                     [Divider]     |
                  |                        |          |
                  +------------------------+----------+
                                           |
                                      RF Output OUT
Quick Start
C++
#include 
#include 
#include 

const int CS_PIN = 10;
ADF4351 synth;

// Helper function to send 32-bit register words over hardware SPI
void writeRegister(uint32_t regData) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer((regData >> 24) & 0xFF);
    SPI.transfer((regData >> 16) & 0xFF);
    SPI.transfer((regData >> 8) & 0xFF);
    SPI.transfer(regData & 0xFF);
    digitalWrite(CS_PIN, HIGH);
}

void setup() {
    Serial.begin(115200);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    SPI.begin();

    // 1. Configure synthesizer parameters (optional step; default is 25MHz ref clock)
    synth.setRefFreqHz(25000000);  // 25 MHz Reference
    synth.setOutputPower(3);       // +5 dBm Output Power

    // 2. Compute registers for target frequency (e.g., 433.92 MHz)
    uint32_t regs[6];
    if (synth.calculateRegisters(433920000ULL, regs)) {
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
API Reference & Keyword Usage
Core Datatypes & Classes
ADF4351
The primary class representing the ADF4351 synthesizer instance.

C++
ADF4351 synth; // Instantiate with default configuration (25MHz TCXO/OCXO ref)
Config
A configuration structure containing all chip settings.

C++
ADF4351::Config myConfig;
myConfig.refFreqHz = 10000000; // Set reference clock to 10 MHz
myConfig.outputPower = 2;      // Set power to +2 dBm
Core Functions
calculateRegisters(freqHz, outRegisters)
Computes the 6 required 32-bit register values for a given frequency in Hertz.

Parameters:

freqHz (uint64_t): Target output frequency in Hz (35,000,000 to 4,400,000,000 Hz).

outRegisters (uint32_t[6]): Array to hold the resulting register words R 
0
​
  through R 
5
​
 .

Returns: bool – true if calculation succeeded; false if frequency is out of range or invalid.

C++
uint32_t registers[6];
bool success = synth.calculateRegisters(1000000000ULL, registers); // 1.0 GHz
Bulk Configuration
setConfig(config)
Overwrites the entire configuration struct in one operation.

C++
ADF4351::Config cfg;
cfg.refFreqHz = 25000000;
cfg.outputPower = 3;
cfg.rfOutputEnable = 1;

synth.setConfig(cfg);
getConfig()
Retrieves the currently loaded configuration struct.

C++
ADF4351::Config activeCfg = synth.getConfig();
Serial.print("Current Ref Freq: ");
Serial.println(activeCfg.refFreqHz);
Register 5 Setters
setRefFreqHz(val)
Sets the reference input frequency in Hz.

C++
synth.setRefFreqHz(25000000); // 25 MHz Reference
setLdPinMode(val)
Configures the Lock Detect (LD) output pin mode (Bits [23:22], Mask: 0x03).

0: Low

1: Digital Lock Detect (Default)

2: Low

3: High

C++
synth.setLdPinMode(1); // Digital Lock Detect
Register 4 Setters
setFeedbackSelect(val)
Selects VCO feedback signal source to N-counter (Bit [23], Mask: 0x01).

0: Divided (VCO output through divider)

1: Fundamental (VCO output directly; Default)

C++
synth.setFeedbackSelect(1);
setVcoPowerdown(val)
Powers down the internal VCO (Bit [11], Mask: 0x01).

0: VCO Active

1: VCO Powered Down

C++
synth.setVcoPowerdown(0);
setMtld(val)
Mute Till Lock Detect (Bit [10], Mask: 0x01).

0: Mute Disabled

1: Mute Enabled (RF output muted until PLL locks)

C++
synth.setMtld(1);
setAuxOutputSelect(val)
Selects auxiliary RF output signal path (Bit [9], Mask: 0x01).

0: Divided Output

1: Fundamental Output

C++
synth.setAuxOutputSelect(0);
setAuxOutputEnable(val)
Enables or disables auxiliary RF output pin (Bit [8], Mask: 0x01).

0: Disabled

1: Enabled

C++
synth.setAuxOutputEnable(0);
setAuxOutputPower(val)
Configures auxiliary RF output power level (Bits [7:6], Mask: 0x03).

0: -4 dBm

1: -1 dBm

2: +2 dBm

3: +5 dBm

C++
synth.setAuxOutputPower(0); // -4 dBm
setRfOutputEnable(val)
Main RF output power toggle (Bit [5], Mask: 0x01).

0: Output Disabled

1: Output Enabled

C++
synth.setRfOutputEnable(1);
setOutputPower(val)
Sets main RF output power level (Bits [4:3], Mask: 0x03).

0: -4 dBm

1: -1 dBm

2: +2 dBm

3: +5 dBm

C++
synth.setOutputPower(3); // +5 dBm maximum power
Register 3 Setters
setBandSelectMode(val)
Configures Band Select Clock logic mode (Bit [23], Mask: 0x01).

0: Low / Standard PFD frequency

1: High / Fast PFD frequency

C++
synth.setBandSelectMode(0);
setAbp(val)
Anti-Backlash Pulse Width (Bit [22], Mask: 0x01).

0: 6 ns (Recommended for Fractional-N operation)

1: 3 ns (Recommended for Integer-N operation)

C++
synth.setAbp(0);
setChargeCancel(val)
Charge pump cancellation control (Bit [21], Mask: 0x01).

0: Disabled

1: Enabled

C++
synth.setChargeCancel(0);
setCsr(val)
Cycle Slip Reduction (Bit [18], Mask: 0x01).

0: Disabled

1: Enabled

C++
synth.setCsr(0);
setClkDivMode(val)
Clock Divider Mode (Bits [16:15], Mask: 0x03).

0: Clock Divider Off

1: Fast Lock Enable

2: Resynchronization Enable

C++
synth.setClkDivMode(0);
setClkDivValue(val)
12-bit clock divider value for fast-lock / resync (Bits [14:3], Mask: 0x0FFF). Range: 0 to 4095.

C++
synth.setClkDivValue(150);
Register 2 Setters
setNoiseMode(val)
Low Noise vs. Low Spur Mode configuration (Bits [30:29], Mask: 0x03).

0: Low Noise Mode

3: Low Spur Mode

C++
synth.setNoiseMode(0); // Low noise mode
setMuxout(val)
Configures the multiplexer output pin function (Bits [28:26], Mask: 0x07).

0: Three-State Output

1: DVNN

2: N-Divider Output

3: R-Divider Output

4: Analog Lock Detect

5: Digital Lock Detect

C++
synth.setMuxout(0);
setReferenceDoubler(val)
Enables input reference frequency multiplier (Bit [25], Mask: 0x01).

0: Disabled

1: Enabled (Doubles PFD frequency)

C++
synth.setReferenceDoubler(0);
setRdiv2(val)
Reference Divide-by-2 toggle (Bit [24], Mask: 0x01).

0: Disabled

1: Divide reference clock by 2 before R-divider

C++
synth.setRdiv2(0);
setRDivider(val)
10-bit Reference R-Divider value (Bits [23:14], Mask: 0x03FF). Range: 1 to 1023.

C++
synth.setRDivider(1);
setDoubleBuff(val)
Double buffering control for Register 4 (Bit [13], Mask: 0x01).

0: Disabled

1: Enabled

C++
synth.setDoubleBuff(0);
setChargePumpCurr(val)
Configures charge pump current setting (Bits [12:9], Mask: 0x0F).

Range: 0 (0.31 mA) to 15 (5.00 mA). Default 7 = 2.50 mA.

C++
synth.setChargePumpCurr(7); // 2.50 mA
setLdf(val)
Lock Detect Functionality selection (Bit [8], Mask: 0x01).

0: Fractional-N Mode Lock Detect

1: Integer-N Mode Lock Detect

C++
synth.setLdf(0);
setLdp(val)
Lock Detect Precision pulse threshold (Bit [7], Mask: 0x01).

0: 10 ns (40 consecutive cycles)

1: 6 ns (40 consecutive cycles)

C++
synth.setLdp(0);
setPdPolarity(val)
Phase Detector Polarity (Bit [6], Mask: 0x01).

0: Negative (Non-inverting loop filter)

1: Positive (Inverting loop filter / active filter)

C++
synth.setPdPolarity(1);
setPowerDown(val)
Software chip power-down mode (Bit [5], Mask: 0x01).

0: Normal Operation

1: Software Power Down

C++
synth.setPowerDown(0);
setCpThreeState(val)
Charge Pump Three-State control (Bit [4], Mask: 0x01).

0: Normal Operation

1: High Impedance State

C++
synth.setCpThreeState(0);
setCounterReset(val)
Resets internal N and R counters (Bit [3], Mask: 0x01).

0: Normal Operation

1: Reset Active

C++
synth.setCounterReset(0);
Register 1 & 0 Setters
setAutoPrescaler(enable)
Enables automated dynamic prescaler selection (4/5 vs. 8/9).

C++
synth.setAutoPrescaler(true);
setPrescaler(val)
Manually overrides the dual-modulus prescaler setting (Bit [27], Mask: 0x01). Automatically disables autoPrescaler.

0: 4/5 Prescaler

1: 8/9 Prescaler

C++
synth.setPrescaler(1); // Manual 8/9 prescaler
setPhaseAdjust(val)
Phase adjustment control toggle (Bit [28], Mask: 0x01).

0: Disabled

1: Enabled

C++
synth.setPhaseAdjust(0);
setPhaseValue(val)
12-bit fractional phase word (Bits [26:15], Mask: 0x0FFF). Range: 0 to 4095.

C++
synth.setPhaseValue(1);
Hardware Files
Complete open-source hardware design files created in KiCad are available in the hardware/ directory:

ADF4351.kicad_sch: Complete schematic diagram with power filtering and RF matching networks.

ADF4351.kicad_pcb: 4-layer controlled impedance PCB layout.

ADF4351.kicad_pro: KiCad project workspace file.

License
This library is released under the MIT License. Free to use, modify, and distribute in both open-source and commercial applications.
