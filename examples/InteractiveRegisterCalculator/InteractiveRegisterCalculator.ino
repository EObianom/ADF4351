/**
 * @file InteractiveRegisterCalculator.ino
 * @brief Interactive Serial utility to calculate ADF4351 32-bit register values in Hex.
 * 
 * @author Dr. Ekenedirichukwu Obianom
 * @date October 2026
 * 
 * Prompts the user via Serial Monitor for a Reference Frequency (Hz) and Target 
 * Output Frequency (Hz), then computes and displays the required 32-bit register 
 * values (R5 down to R0) formatted in padded Hexadecimal notation.
 * 
 * @target Adafruit Metro ESP32-S3 / General Arduino Framework
 * @hardware ADF4351 Wideband Synthesizer Board
 */

#include <Arduino.h>
#include "ADF4351.h"

// Instantiate the class
ADF4351 adf;

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
    Serial.begin(115200);
    while (!Serial) { ; } // Wait for Serial Monitor connection
    
    Serial.println(F("\n========================================"));
    Serial.println(F(" ADF4351 Interactive Hex Register Calculator"));
    Serial.println(F("========================================"));
}

void loop() {
    // Prompt for Reference Frequency
    Serial.println(F("\nEnter Reference Frequency in Hz (e.g. 25000000):"));
    uint64_t refFreq = readUint64FromSerial();
    Serial.print(F("-> Ref Freq set to: "));
    Serial.print((uint32_t)refFreq);
    Serial.println(F(" Hz"));

    adf.setRefFreqHz((uint32_t)refFreq);
    
    // Prompt for Target Output Frequency
    Serial.println(F("Enter Target Output Frequency in Hz (e.g. 433000000):"));
    uint64_t outFreq = readUint64FromSerial();
    Serial.print(F("-> Output Freq set to: "));
    Serial.print((uint32_t)(outFreq / 1000000ULL));
    Serial.print(F(" MHz ("));
    Serial.print((uint32_t)outFreq);
    Serial.println(F(" Hz)"));

    uint32_t registers[6];
    bool success = adf.calculateRegisters(outFreq, registers);

    Serial.println(F("\n----------------------------------------"));
    if (success) {
        Serial.println(F("Calculated Registers (R5 down to R0):"));
        for (int i = 5; i >= 0; i--) {
            Serial.print(F("  R"));
            Serial.print(i);
            Serial.print(F(" = 0x"));
            
            // Format 32-bit uint32_t with leading zeros for standard 8-digit HEX display
            for (int shift = 28; shift >= 0; shift -= 4) {
                uint8_t nibble = (registers[i] >> shift) & 0x0F;
                Serial.print(nibble, HEX);
            }
            Serial.println();
        }
    } else {
        Serial.println(F("ERROR: Out of bounds or invalid frequency combination!"));
        Serial.println(F("Note: RF Output must be between 35 MHz and 4.4 GHz."));
    }
    Serial.println(F("----------------------------------------"));

    delay(1000);
}