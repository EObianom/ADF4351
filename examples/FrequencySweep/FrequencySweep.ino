#include <Arduino.h>
#include <SPI.h>
#include "ADF4351.h"

ADF4351 adf;

// Pin Definitions (Metro ESP32-S3)
#define ADF_PIN_SS   10   // LE (Latch Enable)
#define ADF_PIN_CE   12   // Chip Enable
#define ADF_PIN_MOSI 11   // DAT
#define ADF_PIN_SCK  13   // CLK
#define ADF_PIN_LD    4   // Lock Detect

void writeADF4351Register(uint32_t regValue) {
    digitalWrite(ADF_PIN_SS, LOW);
    SPI.beginTransaction(SPISettings(5000000, MSBFIRST, SPI_MODE0));
    SPI.transfer32(regValue);
    SPI.endTransaction();
    digitalWrite(ADF_PIN_SS, HIGH);
    delayMicroseconds(1);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    pinMode(ADF_PIN_SS, OUTPUT);
    pinMode(ADF_PIN_CE, OUTPUT);
    pinMode(ADF_PIN_LD, INPUT_PULLDOWN);

    digitalWrite(ADF_PIN_SS, HIGH);
    digitalWrite(ADF_PIN_CE, HIGH);

    // Initialize Hardware SPI on ESP32-S3
    SPI.begin(ADF_PIN_SCK, -1, ADF_PIN_MOSI, ADF_PIN_SS);

    Serial.println(F("ADF4351 Optimized Sweep Initialized."));
}

void loop() {
    const uint64_t startFreq = 1500000000ULL; // 1.5 GHz
    const uint64_t stopFreq  = 2700000000ULL; // 2.7 GHz
    const uint64_t stepFreq  =   10000000ULL; // 10 MHz
    const uint64_t refFreq   =   25000000ULL; // 25 MHz
    const uint16_t dwellMs   = 15;

    adf.setRefFreqHz((uint32_t)refFreq);

    Serial.println(F("\n--- Starting Sweep (1.5 GHz -> 2.7 GHz) ---"));

    for (uint64_t currentFreq = startFreq; currentFreq <= stopFreq; currentFreq += stepFreq) {
        uint32_t registers[6];
        bool success = adf.calculateRegisters(currentFreq, registers);

        if (!success) {
            Serial.print(F("Failed to calculate registers for "));
            Serial.print((uint32_t)(currentFreq / 1000000ULL));
            Serial.println(F(" MHz"));
            continue;
        }

        // Send R5 down to R0 to load configuration
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

        Serial.print(F("Freq: "));
        Serial.print((uint32_t)(currentFreq / 1000000ULL));
        Serial.print(F(" MHz | Status: "));
        Serial.println(isLocked ? F("LOCKED") : F("UNLOCKED"));

        delay(dwellMs);
    }

    delay(2000);
}