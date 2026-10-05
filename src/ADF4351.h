/**
 * @file ADF4351.h
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

    /**
    * @brief Calculates the 6 32-bit register values required to lock the ADF4351 to a target RF frequency.
    * 
    * Performs automatic RF output divider selection (VCO 2.2 GHz – 4.4 GHz), PFD frequency division, 
    * integer/fractional divider (INT, FRAC, MOD) reduction via GCD, prescaler selection, and 
    * band select clock calculation before packing the control bits into registers R0 through R5.
    * 
    * @param freqHz Target RF output frequency in Hz (35,000,000 Hz to 4,400,000,000 Hz).
    * @param outRegisters Array of 6 uint32_t elements to store the computed register words [R0..R5].
    * @return true Configuration calculated successfully and frequency is within limits.
    * @return false Target frequency is out of range, PFD frequency is invalid, or divider bounds were exceeded.
    */
    bool calculateRegisters(uint64_t freqHz, uint32_t outRegisters[6]);

    // --- Dynamic Bulk Configuration ---
    /**
     * @brief Replaces the active configuration state with a complete custom Config bundle.
     * @param config ADF4351::Config structure populated with target parameter settings.
     */
    void setConfig(const Config& config) { m_config = config; }

    /**
     * @brief Retrieves a copy of the current active driver configuration structure.
     * @return ADF4351::Config Active configuration state.
     */
    Config getConfig() const { return m_config; }
  
    // --- Dynamic Individual Parameter Setters ---
    // Register 5
    /**
     * @brief Sets the reference oscillator clock frequency.
     * @param val Reference frequency in Hz (e.g., 25000000 for 25 MHz).
     */
    void setRefFreqHz(uint32_t val) { m_config.refFreqHz = val; }

    /**
     * @brief Configures Lock Detect (LD) pin operation mode (Register 5, Bits [23:22]).
     * @param val Mode setting (0 = Low, 1 = Digital Lock Detect, 2 = Low, 3 = High).
     */
    void setLdPinMode(uint32_t val) { m_config.ldPinMode = val & 0x03; }

    // Register 4
    /**
     * @brief Selects feedback signal path to N-divider (Register 4, Bit 23).
     * @param val 0 = Divided signal from RF output divider, 1 = Fundamental VCO signal.
     */
    void setFeedbackSelect(uint32_t val) { m_config.feedbackSelect = val & 0x01; }

    /**
     * @brief Controls VCO powerdown mode (Register 4, Bit 11).
     * @param val 0 = VCO Active, 1 = VCO Powered down.
     */
    void setVcoPowerdown(uint32_t val) { m_config.vcoPowerdown = val & 0x01; }

    /**
     * @brief Enables or disables Mute Till Lock Detect (Register 4, Bit 10).
     * @param val 0 = Disabled, 1 = Mute RF output until lock detect is achieved.
     */
    void setMtld(uint32_t val) { m_config.mtld = val & 0x01; }

    /**
     * @brief Selects signal source for Auxiliary Output (Register 4, Bit 9).
     * @param val 0 = Divided output, 1 = Fundamental VCO output.
     */
    void setAuxOutputSelect(uint32_t val) { m_config.auxOutputSelect = val & 0x01; }

    /**
     * @brief Enables or disables the Auxiliary RF Output port (Register 4, Bit 8).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setAuxOutputEnable(uint32_t val) { m_config.auxOutputEnable = val & 0x01; }

    /**
     * @brief Sets Auxiliary RF Output power level (Register 4, Bits [7:6]).
     * @param val Power index (0 = -4dBm, 1 = -1dBm, 2 = +2dBm, 3 = +5dBm).
     */
    void setAuxOutputPower(uint32_t val) { m_config.auxOutputPower = val & 0x03; }

    /**
     * @brief Enables or disables Main RF Output port (Register 4, Bit 5).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setRfOutputEnable(uint32_t val) { m_config.rfOutputEnable = val & 0x01; }

    /**
     * @brief Sets Main RF Output power level (Register 4, Bits [4:3]).
     * @param val Power index (0 = -4dBm, 1 = -1dBm, 2 = +2dBm, 3 = +5dBm).
     */
    void setOutputPower(uint32_t val) { m_config.outputPower = val & 0x03; }

    // Register 3
    /**
     * @brief Sets Band Select Clock division mode (Register 3, Bit 23).
     * @param val 0 = Low, 1 = High.
     */
    void setBandSelectMode(uint32_t val) { m_config.bandSelectMode = val & 0x01; }

    /**
     * @brief Sets Anti-Backlash Pulse Width (Register 3, Bit 22).
     * @param val 0 = 6.0 ns (Fractional-N mode), 1 = 3.0 ns (Integer-N mode).
     */
    void setAbp(uint32_t val) { m_config.abp = val & 0x01; }

    /**
     * @brief Enables or disables Charge Cancellation (Register 3, Bit 21).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setChargeCancel(uint32_t val) { m_config.chargeCancel = val & 0x01; }

    /**
     * @brief Enables or disables Cycle Slip Reduction (Register 3, Bit 18).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setCsr(uint32_t val) { m_config.csr = val & 0x01; }

    /**
     * @brief Sets Clock Divider operating mode (Register 3, Bits [16:15]).
     * @param val 0 = Off, 1 = Fast Lock, 2 = Resync Enable.
     */
    void setClkDivMode(uint32_t val) { m_config.clkDivMode = val & 0x03; }

    /**
     * @brief Sets 12-bit Clock Divider value (Register 3, Bits [14:3]).
     * @param val Clock divider value (0 to 4095).
     */
    void setClkDivValue(uint32_t val) { m_config.clkDivValue = val & 0x0FFF; }

    // Register 2
    /**
     * @brief Selects Phase Noise vs Spur Mode optimization (Register 2, Bits [30:29]).
     * @param val 0 = Low Noise Mode, 3 = Low Spur Mode.
     */
    void setNoiseMode(uint32_t val) { m_config.noiseMode = val & 0x03; }

    /**
     * @brief Sets MUXOUT output pin multiplexer mode (Register 2, Bits [28:26]).
     * @param val Mode setting (0 = Three-state, 1 = DVdd, 2 = DGND, 3 = R-Divider Output, 
     *                          4 = N-Divider Output, 5 = Analog Lock Detect, 6 = Digital Lock Detect).
     */
    void setMuxout(uint32_t val) { m_config.muxout = val & 0x07; }

    /**
     * @brief Enables or disables Reference Doubler bit (Register 2, Bit 25).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setReferenceDoubler(uint32_t val) { m_config.referenceDoubler = val & 0x01; }

    /**
     * @brief Enables or disables Reference Divide-by-2 bit (Register 2, Bit 24).
     * @param val 0 = Disabled, 1 = Enabled (Halves reference frequency).
     */
    void setRdiv2(uint32_t val) { m_config.rdiv2 = val & 0x01; }

    /**
     * @brief Sets 10-bit Reference R Counter value (Register 2, Bits [23:14]).
     * @param val Division value (1 to 1023).
     */
    void setRDivider(uint32_t val) { m_config.rDivider = val & 0x03FF; }

    /**
     * @brief Enables or disables Double Buffering for Register 4 (Register 2, Bit 13).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setDoubleBuff(uint32_t val) { m_config.doubleBuff = val & 0x01; }

    /**
     * @brief Sets Charge Pump Current setting (Register 2, Bits [12:9]).
     * @param val Current level setting (0 = 0.31 mA to 15 = 5.00 mA).
     */
    void setChargePumpCurr(uint32_t val) { m_config.chargePumpCurr = val & 0x0F; }

    /**
     * @brief Selects Lock Detect Fractional precision (Register 2, Bit 8).
     * @param val 0 = Fractional-N Mode, 1 = Integer-N Mode.
     */
    void setLdf(uint32_t val) { m_config.ldf = val & 0x01; }

    /**
     * @brief Selects Lock Detect Phase precision (Register 2, Bit 7).
     * @param val 0 = 10 ns, 1 = 6 ns.
     */
    void setLdp(uint32_t val) { m_config.ldp = val & 0x01; }

    /**
     * @brief Sets Phase Detector Polarity (Register 2, Bit 6).
     * @param val 0 = Negative, 1 = Positive.
     */
    void setPdPolarity(uint32_t val) { m_config.pdPolarity = val & 0x01; }

    /**
     * @brief Controls Software Power-Down state (Register 2, Bit 5).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setPowerDown(uint32_t val) { m_config.powerDown = val & 0x01; }

    /**
     * @brief Sets Charge Pump output mode (Register 2, Bit 4).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setCpThreeState(uint32_t val) { m_config.cpThreeState = val & 0x01; }

    /**
     * @brief Controls Counter Reset state (Register 2, Bit 3).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setCounterReset(uint32_t val) { m_config.counterReset = val & 0x01; }

    // Register 1 & 0
    /**
     * @brief Enables or disables Automatic Prescaler calculation.
     * @param enable true = Automatically select 4/5 or 8/9 prescaler, false = Use manual setting.
     */
    void setAutoPrescaler(bool enable) { m_config.autoPrescaler = enable; }

    /**
     * @brief Manually selects the Prescaler value and disables auto-prescaler selection (Register 1, Bit 27).
     * @param val 0 = 4/5 prescaler, 1 = 8/9 prescaler.
     */
    void setPrescaler(uint32_t val) { m_config.prescaler = val & 0x01; m_config.autoPrescaler = false; }

    /**
     * @brief Enables or disables Phase Adjustment (Register 1, Bit 28).
     * @param val 0 = Disabled, 1 = Enabled.
     */
    void setPhaseAdjust(uint32_t val) { m_config.phaseAdjust = val & 0x01; }

    /**
     * @brief Sets 12-bit Phase Value (Register 1, Bits [26:15]).
     * @param val Phase word setting (0 to 4095).
     */
    void setPhaseValue(uint32_t val) { m_config.phaseValue = val & 0x0FFF; }

private:
    Config m_config;

    /**
    * @brief Computes the Greatest Common Divisor (GCD) using the Euclidean algorithm.
    * 
    * Used internally to simplify fractional divider ratios (MOD and FRAC) for 
    * ADF4351 register calculations.
    * 
    * @param a First unsigned 32-bit integer.
    * @param b Second unsigned 32-bit integer.
    * @return uint32_t Greatest common divisor of \p a and \p b.
    */
    static uint32_t gcd(uint32_t a, uint32_t b);
};

#endif // ADF4351_H