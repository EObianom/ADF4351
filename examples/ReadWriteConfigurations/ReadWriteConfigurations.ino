/**
 * @file ReadWriteConfiguration.ino
 * @brief Configuration bundle example for the ADF4351 RF Synthesizer.
 * 
 * @author Dr. Ekenedirichukwu Obianom
 * @date October 2026
 * 
 * Demonstrates how to read default chip configuration parameters, build a custom
 * ADF4351::Config struct bundle, apply it using setConfig(), and verify the
 * updated settings.
 * 
 * @target Adafruit Metro ESP32-S3 / General Arduino Framework
 * @hardware ADF4351 Wideband Synthesizer Board
 */
 
#include <Arduino.h>
#include "ADF4351.h"

// Instantiate the class
ADF4351 adf;

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    while (!Serial) { ; }   // Wait for Serial monitor to connect

    Serial.println(F("=== 1. Reading Default Configuration ==="));
    
    // Read the current configuration from the driver
    ADF4351::Config currentConfig = adf.getConfig();
    
    // Print out a few default values
    Serial.print(F("Default Ref Freq: "));
    Serial.print(currentConfig.refFreqHz);
    Serial.println(F(" Hz"));
    
    Serial.print(F("Default Output Power: "));
    Serial.println(currentConfig.outputPower); // 3 = +5dBm by default


    Serial.println(F("\n=== 2. Modifying & Applying New Configuration ==="));

    // Create a new Config bundle
    ADF4351::Config customSettings;

    // Tweak specific settings in our custom bundle
    customSettings.refFreqHz   = 10000000; // Change reference clock to 10 MHz
    customSettings.outputPower = 1;        // Set RF output power to -1 dBm
    customSettings.ldPinMode   = 3;        // Set Lock Detect pin to High
    customSettings.rDivider    = 2;        // Change reference divider to 2

    // Apply the entire bundle to the driver using setConfig
    adf.setConfig(customSettings);

    Serial.println(F("=== 3. Verifying Updated Configuration ==="));

    // Read the updated configuration back from the driver
    ADF4351::Config updatedConfig = adf.getConfig();

    Serial.print(F("Updated Ref Freq: "));
    Serial.print(updatedConfig.refFreqHz);
    Serial.println(F(" Hz"));

    Serial.print(F("Updated Output Power: "));
    Serial.println(updatedConfig.outputPower);

    Serial.print(F("Updated R Divider: "));
    Serial.println(updatedConfig.rDivider);

 /*
     * Alternative inline struct initialization option:
     *
     * adf.setConfig(ADF4351::Config{
     *     .refFreqHz = 10000000,
     *     .outputPower = 1,
     *     .rDivider = 2
     * });
     */
}

void loop() {
    // Idle - setup runs configuration demonstration once
}