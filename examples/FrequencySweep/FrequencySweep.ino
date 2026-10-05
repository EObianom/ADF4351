/**
 * @file FrequencySweep.ino
 * @brief Frequency sweep example for the ADF4351 RF Synthesizer using ESP32-S3.
 * 
 * @author Dr. Ekenedirichukwu Obianom 
 * @date October 2026
 * 
 * @target Adafruit Metro ESP32-S3
 * @hardware ADF4351 Wideband Synthesizer Board
 * 
 * Pin Connections:
 *   ADF4351 LE  (Latch Enable) -> GPIO 10 (CS)
 *   ADF4351 DAT (MOSI)         -> GPIO 11 (MOSI)
 *   ADF4351 CE  (Chip Enable)  -> GPIO 12 (CE)
 *   ADF4351 CLK (SCK)          -> GPIO 13 (SCK)
 *   ADF4351 LD  (Lock Detect)  -> GPIO 4  (Input Pull-down)
 */

#include <Arduino.h>
#include <SPI.h>
#include "ADF4351.h"

// Instantiate the class
ADF4351 adf;

// Pin Definitions (Metro ESP32-S3)
#define ADF_PIN_CS   10   // LE (Latch Enable)
#define ADF_PIN_CE   12   // Chip Enable
#define ADF_PIN_MOSI 11   // DAT
#define ADF_PIN_SCK  13   // CLK
#define ADF_PIN_LD    4   // Lock Detect

// These are mutable values that can be changed depending on needs
const uint64_t startFreq =  500000000ULL; // 0.5 GHz
const uint64_t stopFreq  = 3000000000ULL; // 3.0 GHz
const uint64_t stepFreq  =   10000000ULL; // 10 MHz
const uint64_t refFreq   =   25000000ULL; // 25 MHz
const uint16_t dwellMs   = 15;

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

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    while (!Serial) { ; }

    // Initilize Pin Modes
    pinMode(ADF_PIN_CS, OUTPUT);
    pinMode(ADF_PIN_CE, OUTPUT);
    pinMode(ADF_PIN_LD, INPUT_PULLDOWN);

    // Start Pins as high to enable chip but prevent communication
    digitalWrite(ADF_PIN_CS, HIGH);
    digitalWrite(ADF_PIN_CE, HIGH);

    // Initialize Hardware SPI on ESP32-S3
    SPI.begin(ADF_PIN_SCK, -1, ADF_PIN_MOSI, ADF_PIN_CS);

    // Setting reference frequency depending on the input oscillator to ADF4351 chip
    adf.setRefFreqHz((uint32_t)refFreq);

    Serial.println(F("ADF4351 Optimized Sweep Initialized."));
}

void loop() {
    Serial.println(F("\n--- Starting Sweep (0.5 GHz -> 3.0 GHz) ---"));

    for (uint64_t currentFreq = startFreq; currentFreq <= stopFreq; currentFreq += stepFreq) {
        uint32_t registers[6];
        bool success = adf.calculateRegisters(currentFreq, registers);

        // Check that the calculations were successful
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

        // Print status
        Serial.print(F("Freq: "));
        Serial.print((uint32_t)(currentFreq / 1000000ULL));
        Serial.print(F(" MHz | Status: "));
        Serial.println(isLocked ? F("LOCKED") : F("UNLOCKED"));

        delay(dwellMs);
    }

    delay(2000);
}