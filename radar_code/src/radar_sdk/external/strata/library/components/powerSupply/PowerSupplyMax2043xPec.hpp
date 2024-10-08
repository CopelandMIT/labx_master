#pragma once

#include <components/Registers8bitPec.hpp>
#include <components/interfaces/IPowerSupplyMax2043x.hpp>


///
/// Implementation to support on-board PMICs (Power-Management-IC) of type MAX2043x
/// which can be configured via I2C using PEC (packet error checking)
///
class PowerSupplyMax2043xPec :
    public IPowerSupplyMax2043x
{
public:
    PowerSupplyMax2043xPec(II2c *accessI2c, uint16_t devAddrI2c);

    ///
    /// @copydoc IPowerSupplyMax2043x::getIRegisters
    ///
    IRegisters<uint8_t> *getIRegisters() override;

    ///
    /// @copydoc IPowerSupplyMax2043x::switchVoltageOutput
    ///
    bool switchVoltageOutput(uint8_t output, bool enable) override;

    ///
    /// @copydoc IPowerSupplyMax2043x::setVoltageOutput
    ///
    bool setVoltageOutput(uint8_t output, uint16_t mV) override;

    ///
    /// @copydoc IPowerSupplyMax2043x::mapVoltageToReset
    ///
    bool mapVoltageToReset(uint8_t voltage, bool active) override;

protected:
    ///
    /// \brief Register map of the PMIC MAX2043x
    ///
    enum PmicRegister : uint8_t
    {
        CID     = 0x00,
        CONFIG1 = 0x01,
        CONFIGE = 0x03,
        FPSCFG  = 0x05,
        PINMAP1 = 0x07,
        STATD   = 0x0B,
        VOUT2   = 0x0E,
        VOUT4   = 0x0F,
        VIN5    = 0x10,
        VIN6    = 0x11,
        WDCFG2  = 0x14,
        WDPROT  = 0x16,
    };

    Registers8bitPec m_registers;

    ///
    /// \brief Get the offset for calculating the register value from the voltage for VOUT
    /// \note The offset is different for different PMIC types. Therefore a derived (specific) class must provide it.
    /// \retval The offset in mV
    ///
    virtual uint16_t getVoutVoltageOffset() = 0;
};
