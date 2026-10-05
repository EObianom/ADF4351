#include "ADF4351.h"
ADF4351::ADF4351() : m_config() {}
ADF4351::ADF4351(Config config) : m_config(config) {}

/**
 * @brief Helper function: Computes Greatest Common Divisor (GCD) via Euclidean algorithm.
 */
uint32_t ADF4351::gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * @brief Computes register settings (R0-R5) for a requested RF frequency.
 * 
 * Step 1: Determines the RF divider ratio to bring VCO into range (2.2–4.4 GHz).
 * Step 2: Calculates INT, FRAC, and MOD parameters using Euclidean GCD scaling.
 * Step 3: Packs configuration parameters into the 6 32-bit ADF4351 register words.
 */
bool ADF4351::calculateRegisters(uint64_t freqHz, uint32_t outRegisters[6]) {
    if (freqHz < 35000000ULL || freqHz > 4400000000ULL) return false;

    // 1. Calculate RF Output Divider (VCO: 2.2 GHz - 4.4 GHz)
    uint8_t rfDivider = 0; // 0 = /1, 1 = /2, 2 = /4, 3 = /8, 4 = /16, 5 = /32, 6 = /64
    uint64_t vcoFreqHz = freqHz;

    while (vcoFreqHz < 2200000000ULL) {
        vcoFreqHz <<= 1;
        rfDivider++;
        if (rfDivider > 6) return false;
    }

    // 2. Compute INT, FRAC, MOD against PFD
    uint32_t pfdFreq = m_config.refFreqHz / m_config.rDivider;
    if (pfdFreq == 0) return false;

    uint32_t intValue = vcoFreqHz / pfdFreq;
    uint32_t remainder = vcoFreqHz % pfdFreq;
    uint32_t fracValue = 0;
    uint32_t modValue = 2; // Default Integer-N MOD

      //Calculate dynamic FRAC and MOD if a remainder exists
    if (remainder != 0) {
        fracValue = remainder;
        modValue = pfdFreq;

        uint32_t commonDivisor = gcd(fracValue, modValue);
        fracValue /= commonDivisor;
        modValue /= commonDivisor;

        // Scale down if MOD exceeds 12-bit limit (4095)
        while (modValue > 4095) {
            fracValue >>= 1;
            modValue  >>= 1;
        }

        if (modValue < 2) modValue = 2; // Minimum allowable MOD for fractional mode
    }

    // Min INT constraint
    if (intValue < 23) return false;

    // Prescaler Selection Rule
    uint8_t prescaler = 0;
    if (m_config.autoPrescaler) { // Auto-calculate rule
        if (vcoFreqHz >= 3000000000ULL || intValue >= 75) {
            prescaler = 1; // 8/9
        } else {
            prescaler = 0; // 4/5
        }
    } else { // Manual override
        prescaler = m_config.prescaler & 0x01;
    }
    // Ensure N >= 75 when using 8/9 prescaler, or N >= 23 when using 4/5
    if (prescaler == 1 && intValue < 75) return false; 
    if (prescaler == 0 && intValue < 23) return false;
    
    // Band Select Clock Divider (Target 125 kHz max)
    uint32_t bsCounter = (pfdFreq + 124999UL) / 125000UL;
    if (bsCounter > 255) bsCounter = 255;

    // 3. Assemble Registers
    // Register 5
    outRegisters[5] = ((m_config.ldPinMode & 0x03) << 22)
                    | ((3 & 0x03) << 19) // Reserved bits
                    | 5;

    // Register 4
    outRegisters[4] = ((m_config.feedbackSelect & 0x01) << 23)
                    | (((uint32_t)rfDivider & 0x07) << 20)
                    | ((bsCounter & 0xFF) << 12)
                    | ((m_config.vcoPowerdown & 0x01) << 11)
                    | ((m_config.mtld & 0x01) << 10)
                    | ((m_config.auxOutputSelect & 0x01) << 9)
                    | ((m_config.auxOutputEnable & 0x01) << 8)
                    | ((m_config.auxOutputPower & 0x03) << 6)
                    | ((m_config.rfOutputEnable & 0x01) << 5)
                    | ((m_config.outputPower & 0x03) << 3)
                    | 4;

    // Register 3
    outRegisters[3] = ((m_config.bandSelectMode & 0x01) << 23)
                    | ((m_config.abp & 0x01) << 22)
                    | ((m_config.chargeCancel & 0x01) << 21)
                    | ((m_config.csr & 0x01) << 18)
                    | ((m_config.clkDivMode & 0x03) << 15)
                    | ((m_config.clkDivValue & 0x0FFF) << 3)
                    | 3;

    // Register 2
    outRegisters[2] = ((m_config.noiseMode & 0x03) << 29)
                    | ((m_config.muxout & 0x07) << 26)
                    | ((m_config.referenceDoubler & 0x01) << 25)
                    | ((m_config.rdiv2 & 0x01) << 24)
                    | ((m_config.rDivider & 0x03FF) << 14)
                    | ((m_config.doubleBuff & 0x01) << 13)
                    | ((m_config.chargePumpCurr & 0x0F) << 9)
                    | ((m_config.ldf & 0x01) << 8)
                    | ((m_config.ldp & 0x01) << 7)
                    | ((m_config.pdPolarity & 0x01) << 6)
                    | ((m_config.powerDown & 0x01) << 5)
                    | ((m_config.cpThreeState & 0x01) << 4)
                    | ((m_config.counterReset & 0x01) << 3)
                    | 2;

    // Register 1
    outRegisters[1] = ((m_config.phaseAdjust & 0x01) << 28)
                    | (((uint32_t)prescaler & 0x01) << 27)
                    | ((m_config.phaseValue & 0x0FFF) << 15)
                    | ((modValue & 0x0FFF) << 3)
                    | 1;

    // Register 0
    outRegisters[0] = ((intValue & 0xFFFF) << 15)
                    | ((fracValue & 0x0FFF) << 3)
                    | 0;

    return true;
}