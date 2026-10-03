#include <Arduino.h>
#include "ADF4351.h"

ADF4351 adf;

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
    while (!Serial) {
        ;
    }
    
    Serial.println(F("\n========================================"));
    Serial.println(F(" ADF4351 Interactive Hex Register Calculator"));
    Serial.println(F("========================================"));
}

void loop() {
    Serial.println(F("\nEnter Reference Frequency in Hz (e.g. 25000000):"));
    uint64_t refFreq = readUint64FromSerial();
    Serial.print(F("-> Ref Freq set to: "));
    Serial.print((uint32_t)refFreq);
    Serial.println(F(" Hz"));

    adf.setRefFreqHz((uint32_t)refFreq);

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
            
            if (registers[i] < 0x10000000UL) Serial.print(F("0"));
            if (registers[i] < 0x01000000UL) Serial.print(F("0"));
            if (registers[i] < 0x00100000UL) Serial.print(F("0"));
            if (registers[i] < 0x00010000UL) Serial.print(F("0"));
            if (registers[i] < 0x00001000UL) Serial.print(F("0"));
            if (registers[i] < 0x00000100UL) Serial.print(F("0"));
            if (registers[i] < 0x00000010UL) Serial.print(F("0"));
            
            Serial.println(registers[i], HEX);
        }
    } else {
        Serial.println(F("ERROR: Out of bounds or invalid frequency combination!"));
        Serial.println(F("Note: RF Output must be between 35 MHz and 4.4 GHz."));
    }
    Serial.println(F("----------------------------------------"));

    delay(1000);
}