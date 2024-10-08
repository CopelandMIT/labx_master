/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "RemoteVendorCommands.hpp"
#include <components/Registers.hpp>
#include <platform/interfaces/IVendorCommands.hpp>


template <typename AddressType, typename ValueType = AddressType>
class RemoteRegisters :
    public Registers<AddressType, ValueType>
{
public:
    using typename IRegisters<AddressType, ValueType>::BatchType;

    RemoteRegisters(AddressType increment, IVendorCommands *commands, uint16_t wType, uint8_t id);

    ValueType read(AddressType address) override;
    void read(AddressType address, ValueType &value) override;
    void write(AddressType address, ValueType value) override;
    void read(AddressType address, AddressType count, ValueType values[]) override;
    void write(AddressType address, AddressType count, const ValueType values[]) override;

    void readBatch(const AddressType addresses[], AddressType count, ValueType values[]) override;

    void setBits(AddressType address, ValueType bitmask) override;
    void clearBits(AddressType address, ValueType bitmask) override;
    void modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask) override;

protected:
    void writeBatchImpl(const BatchType vals[], AddressType count) override;

    RemoteVendorCommands m_commands;
};


extern template class RemoteRegisters<uint8_t>;
extern template class RemoteRegisters<uint8_t, uint16_t>;
extern template class RemoteRegisters<uint8_t, uint32_t>;
extern template class RemoteRegisters<uint16_t, uint32_t>;
extern template class RemoteRegisters<uint16_t>;
