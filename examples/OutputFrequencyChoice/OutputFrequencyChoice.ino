/**
 * @file OutputFrequencyChoice.ino
 * @brief Interactive Serial utility to dynamically select and output an RF frequency.
 * 
 * @author Dr. Ekenedirichukwu Obianom
 * @date October 2026
 * 
 * Prompts the user via Serial Monitor for a Reference Frequency (Hz) and a Target 
 * RF Output Frequency (Hz), calculates the ADF4351 register values, transmits 
 * them over hardware SPI (R5 down to R0), and checks the Lock Detect (LD) pin 
 * to confirm PLL lock status.
 * 
 * @target Adafruit Metro ESP32-S3 / General Arduino Framework
 * @hardware ADF4351 Wideband Synthesizer Board
 * 
 * Pin Connections:
 *   ADF4351 LE  (Latch Enable) -> GPIO 10 (CS/SS)
 *   ADF4351 DAT (MOSI)         -> GPIO 11 (MOSI)
 *   ADF4351 CE  (Chip Enable)  -> GPIO 12 (CE)
 *   ADF4351 CLK (SCK)          -> GPIO 13 (SCK)
 *   ADF4351 LD  (Lock Detect)  -> GPIO 4  (Input Pull-down)
 */

#include <Arduino.h>
#include <SPI.h>
#include "ADF4351.h"

// Instantiate the driver
ADF4351 adf;

// Pin Definitions (Metro ESP32-S3)
#define ADF_PIN_CS   10   // LE (Latch Enable)
#define ADF_PIN_CE   12   // Chip Enable
#define ADF_PIN_MOSI 11   // DAT
#define ADF_PIN_SCK  13   // CLK
#define ADF_PIN_LD    4   // Lock Detect

/**
 * @brief Writes a 32-bit register value to the ADF4351 via SPI.
 * @param regValue Complete 32-bit word containing register configuration and control bits.
 */
void writeADF4351Register(uint32_t regValue) {
    digitalWrite(ADF_PIN_CS, LOW);
    SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    SPI.transfer32(regValue);
    SPI.endTransaction();
    digitalWrite(ADF_PIN_CS, HIGH);
    delayMicroseconds(1);
}

/**
 * @brief Reads an unsigned 64-bit integer from the Serial input stream.
 * @return Parsed uint64_t numerical value entered by the user.
 */
uint64_t readUint64FromSerial() {
    String inputString = "";
    while (true) {
        while (Serial.available() > 0) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (inputString.length() > 0) {
                    uint64_t value = 0;
                    for (size_t i = 0; i < inputString.length(); i++) {
                        if (isDigit(inputString[i])) {
                            value = (value * 10) + (inputString[i] - '0');
                        }
                    }
                    return value;
                }
            } else if (isDigit(c)) {
                inputString += c;
            }
        }
    }
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for Serial Monitor connection

    // Initialize Pin Modes
    pinMode(ADF_PIN_CS, OUTPUT);
    pinMode(ADF_PIN_CE, OUTPUT);
    pinMode(ADF_PIN_LD, INPUT_PULLDOWN);

    // Set CS and CE high to initialize state
    digitalWrite(ADF_PIN_CS, HIGH);
    digitalWrite(ADF_PIN_CE, HIGH);

    // Initialize Hardware SPI on ESP32-S3
    SPI.begin(ADF_PIN_SCK, -1, ADF_PIN_MOSI, ADF_PIN_CS);

    Serial.println(F("\n========================================"));
    Serial.println(F(" ADF4351 Interactive Frequency Choice"));
    Serial.println(F("========================================"));
}

void loop() {
    // Prompt for Reference Frequency
    Serial.println(F("\nEnter Reference Frequency in Hz (e.g. 25000000 for 25MHz):"));
    uint64_t refFreq = readUint64FromSerial();
    Serial.print(F("-> Ref Freq set to: "));
    Serial.print((uint32_t)refFreq);
    Serial.println(F(" Hz"));

    adf.setRefFreqHz((uint32_t)refFreq);

    // Prompt for Target Output Frequency
    Serial.println(F("Enter Target Output Frequency in Hz (e.g. 1500000000 for 1.5 GHz):"));
    uint64_t outFreq = readUint64FromSerial();
    Serial.print(F("-> Target Output Freq: "));
    Serial.print((uint32_t)(outFreq / 1000000ULL));
    Serial.print(F(" MHz ("));
    Serial.print((uint32_t)outFreq);
    Serial.println(F(" Hz)"));

    uint32_t registers[6];
    bool success = adf.calculateRegisters(outFreq, registers);

    Serial.println(F("\n----------------------------------------"));
    if (success) {
        Serial.println(F("Transmitting Registers to ADF4351 (R5 down to R0)..."));
        
        // Write calculated registers to hardware via SPI
        for (int i = 5; i >= 0; i--) {
            writeADF4351Register(registers[i]);
        }

        delayMicroseconds(500); // Settling delay for loop filter

        // Read physical Lock Detect Pin
        bool isLocked = false;
        for (int retry = 0; retry < 10; retry++) {
            if (digitalRead(ADF_PIN_LD) == HIGH) {
                isLocked = true;
                break;
            }
            delayMicroseconds(100);
        }

        Serial.print(F("PLL Lock Status: "));
        Serial.println(isLocked ? F("LOCKED [SUCCESS]") : F("UNLOCKED [FAILED]"));
        
        if (isLocked) {
            Serial.println(F("RF Output is active and stable at target frequency."));
        } else {
            Serial.println(F("WARNING: PLL failed to lock! Check reference clock, power, or loop filter."));
        }
    } else {
        Serial.println(F("ERROR: Out of bounds or invalid frequency combination!"));
        Serial.println(F("Note: RF Output must be between 35 MHz and 4.4 GHz."));
    }
    Serial.println(F("----------------------------------------"));

    delay(1000);
}