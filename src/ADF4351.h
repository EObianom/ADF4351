/**
 * @file ADF4351.h
 * @brief Driver and Register Calculator for the ADF4351 Wideband Synthesizer.
 * @details Provides a structured interface to compute and format 32-bit register
 *          values (R0–R5) for the Analog Devices ADF4351 Wideband Synthesizer
 *          with an integrated Voltage Controlled Oscillator (VCO). Supports
 *          both Fractional-N and Integer-N Phase-Locked Loop (PLL) operating modes,
 *          dynamic RF/PFD divider scaling, and custom prescaler selection.
 *
 * @author Dr. Ekenedirichukwu Obianom
 * @date 2026
 */

#ifndef ADF4351_H
#define ADF4351_H

#include <Arduino.h>

class ADF4351 {
public:
    // Core Configuration Settings (Defaults match your current configuration)
    struct Config {
        // --- REGISTER 5 PARAMETERS ---
        uint32_t refFreqHz          = 25000000; // 25 MHz Reference Clock
        uint32_t ldPinMode          = 1;        // DB[23:22] (0 = Low, 1 = Digital Lock Detect, 2 = Low, 3 = High)
        
        // --- REGISTER 4 PARAMETERS ---
        uint32_t feedbackSelect     = 1;        // DB[23]    (0 = Divided, 1 = Fundamental)
        uint32_t vcoPowerdown       = 0;        // DB[11]    (0 = VCO powered up, 1 = VCO powered down)
        uint32_t mtld               = 0;        // DB[10]    (0 = Mute till Lock Detect disabled, 1 = Enabled)
        uint32_t auxOutputSelect    = 0;        // DB[9]     (0 = Divided, 1 = Fundamental)
        uint32_t auxOutputEnable    = 0;        // DB[8]     (0 = Disabled, 1 = Enabled)
        uint32_t auxOutputPower     = 0;        // DB[7:6]   (0 = -4dBm, 1 = -1dBm, 2 = +2dBm, 3 = +5dBm)
        uint32_t rfOutputEnable     = 1;        // DB[5]     (0 = Disabled, 1 = Enabled)
        uint32_t outputPower        = 3;        // DB[4:3]   (0 = -4dBm, 1 = -1dBm, 2 = +2dBm, 3 = +5dBm)
        
        // --- REGISTER 3 PARAMETERS ---
        uint32_t bandSelectMode     = 0;        // DB[23]    (0 = Low / standard PFD, 1 = High / fast PFD)
        uint32_t abp                = 0;        // DB[22]    ((0 = 6ns for Frac-N, 1 = 3ns for Int-N)
        uint32_t chargeCancel       = 0;        // DB[21]    (0 = Disabled, 1 = Enabled)
        uint32_t csr                = 0;        // DB[18]    (0 = Disabled, 1 = Enabled)
        uint32_t clkDivMode         = 0;        // DB[16:15] (0 = Off, 1 = Fast Lock, 2 = Resync)
        uint32_t clkDivValue        = 150;      // DB[14:3]  (12-bit clock divider value, 0 to 4095)
        
        // --- REGISTER 2 PARAMETERS ---
        uint32_t noiseMode          = 0;        // DB[30:29] (0 = Low Noise, 3 = Low Spur)
        uint32_t muxout             = 0;        // DB[28:26] (0 = 3-State Output)
        uint32_t referenceDoubler   = 0;        // DB[25]    (0 = Disabled, 1 = Enabled)
        uint32_t rdiv2              = 0;        // DB[24]    (0 = Disabled, 1 = Divide-by-2)
        uint32_t rDivider           = 1;        // DB[23:14] (10-bit R divider, 1 to 1023)
        uint32_t doubleBuff         = 0;        // DB[13]    (0 = Disabled, 1 = Enabled)
        uint32_t chargePumpCurr     = 7;        // DB[12:9]  (7 = 2.50 mA)
        uint32_t ldf                = 0;        // DB[8]     (0 = Frac-N, 1 = INT-N)
        uint32_t ldp                = 0;        // DB[7]     (0 = 10ns, 1 = 6ns)
        uint32_t pdPolarity         = 1;        // DB[6]     (0 = Negative, 1 = Positive)
        uint32_t powerDown          = 0;        // DB[5]     (0 = Disabled, 1 = Enabled)
        uint32_t cpThreeState       = 0;        // DB[4]     (0 = Disabled, 1 = Enabled)
        uint32_t counterReset       = 0;        // DB[3]     (0 = Disabled, 1 = Enabled)
        
        // --- REGISTER 1 PARAMETERS ---
        bool autoPrescaler          = false;     // true = auto-select (4/5 or 8/9), false = use manual prescaler
        uint32_t prescaler          = 1;        // DB[27]    (0 = 4/5, 1 = 8/9)
        uint32_t phaseAdjust        = 0;        // DB[28]    (0 = OFF, 1 = ON)
        uint32_t phaseValue         = 1;        // DB[26:15] (12-bit phase value, typically 1)
    };

    // Constructors
    ADF4351();
    ADF4351(Config config);

    // Primary Calculation Function (populates 6-element array)
    bool calculateRegisters(uint64_t freqHz, uint32_t outRegisters[6]);

    // Dynamic Bulk Configuration
    void setConfig(const Config& config) { m_config = config; }
    Config getConfig() const { return m_config; }

    // --- Dynamic Individual Parameter Setters ---
    void setRefFreqHz(uint32_t val)       { m_config.refFreqHz = val; }
    void setLdPinMode(uint32_t val)       { m_config.ldPinMode = val & 0x03; }

    void setFeedbackSelect(uint32_t val)  { m_config.feedbackSelect = val & 0x01; }
    void setVcoPowerdown(uint32_t val)    { m_config.vcoPowerdown = val & 0x01; }
    void setMtld(uint32_t val)            { m_config.mtld = val & 0x01; }
    void setAuxOutputSelect(uint32_t val) { m_config.auxOutputSelect = val & 0x01; }
    void setAuxOutputEnable(uint32_t val) { m_config.auxOutputEnable = val & 0x01; }
    void setAuxOutputPower(uint32_t val)  { m_config.auxOutputPower = val & 0x03; }
    void setRfOutputEnable(uint32_t val)  { m_config.rfOutputEnable = val & 0x01; }
    void setOutputPower(uint32_t val)     { m_config.outputPower = val & 0x03; }

    void setBandSelectMode(uint32_t val)  { m_config.bandSelectMode = val & 0x01; }
    void setAbp(uint32_t val)             { m_config.abp = val & 0x01; }
    void setChargeCancel(uint32_t val)    { m_config.chargeCancel = val & 0x01; }
    void setCsr(uint32_t val)             { m_config.csr = val & 0x01; }
    void setClkDivMode(uint32_t val)      { m_config.clkDivMode = val & 0x03; }
    void setClkDivValue(uint32_t val)     { m_config.clkDivValue = val & 0x0FFF; }
    
    void setNoiseMode(uint32_t val)       { m_config.noiseMode = val & 0x03; }
    void setMuxout(uint32_t val)          { m_config.muxout = val & 0x07; }
    void setReferenceDoubler(uint32_t val){ m_config.referenceDoubler = val & 0x01; }
    void setRdiv2(uint32_t val)           { m_config.rdiv2 = val & 0x01; }
    void setRDivider(uint32_t val)        { m_config.rDivider = val & 0x03FF; }
    void setDoubleBuff(uint32_t val)      { m_config.doubleBuff = val & 0x01; }
    void setChargePumpCurr(uint32_t val)  { m_config.chargePumpCurr = val & 0x0F; }
    void setLdf(uint32_t val)             { m_config.ldf = val & 0x01; }
    void setLdp(uint32_t val)             { m_config.ldp = val & 0x01; }
    void setPdPolarity(uint32_t val)      { m_config.pdPolarity = val & 0x01; }
    void setPowerDown(uint32_t val)       { m_config.powerDown = val & 0x01; }
    void setCpThreeState(uint32_t val)    { m_config.cpThreeState = val & 0x01; }
    void setCounterReset(uint32_t val)    { m_config.counterReset = val & 0x01; }

    void setAutoPrescaler(bool enable)  { m_config.autoPrescaler = enable; }
    void setPrescaler(uint32_t val)     { m_config.prescaler = val & 0x01; m_config.autoPrescaler = false; }
    void setPhaseAdjust(uint32_t val)     { m_config.phaseAdjust = val & 0x01; }
    void setPhaseValue(uint32_t val)      { m_config.phaseValue = val & 0x0FFF; }

private:
    Config m_config;
    static uint32_t gcd(uint32_t a, uint32_t b);
};

#endif // ADF4351_H