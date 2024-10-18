/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RemoteRegisters.hpp"
#include <algorithm>
#include <common/Buffer.hpp>
#include <common/Numeric.hpp>
#include <common/Serialization.hpp>
#include <universal/components/subinterfaces.h>
#include <universal/components/subinterfaces/iregisters.h>
#include <universal/protocol/protocol_definitions.h>
#include <vector>


template <typename AddressType, typename ValueType>
RemoteRegisters<AddressType, ValueType>::RemoteRegisters(AddressType increment, IVendorCommands *commands, uint16_t wType, uint8_t id) :
    Registers<AddressType, ValueType>(increment),
    m_commands {commands, CMD_COMPONENT, wType, id, COMPONENT_SUBIF_REGISTERS}
{
}

template <typename AddressType, typename ValueType>
ValueType RemoteRegisters<AddressType, ValueType>::read(AddressType address)
{
    ValueType value;
    read(address, 1, &value);
    return value;
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::read(AddressType address, ValueType &value)
{
    read(address, 1, &value);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::write(AddressType address, ValueType value)
{
    write(address, 1, &value);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::read(AddressType address, AddressType count, ValueType values[])
{
    const auto increment = Registers<AddressType, ValueType>::m_increment;

    if (m_commands.getProtocolVersion() < 0x00040000)  // STRATA_LEGACY_PROTOCOL_3
    {
        constexpr uint16_t elemSize = sizeof(values[0]);
        constexpr uint16_t argSize  = sizeof(address) + sizeof(count);
        uint8_t payload[argSize];
        const auto maxCount = clampValue<decltype(count)>(m_commands.getMaxTransfer() / elemSize);

        while (count > 0)
        {
            const decltype(count) wCount = std::min(count, maxCount);
            const uint16_t wLength       = wCount * elemSize;
            uint8_t *it                  = payload;
            it                           = hostToSerial(it, address);
            it                           = hostToSerial(it, wCount);

            m_commands.vendorTransferChecked(FN_REGISTERS_READ_BURST, argSize, payload, wLength, reinterpret_cast<uint8_t *>(values));

            address += (wCount * increment);
            values += wCount;
            count -= wCount;
        }
        return;
    }

    auto updateParams = [&address, &increment](decltype(count) wCount) {
        address += (wCount * increment);
    };

    m_commands.vendorTransferMultiRead(FN_REGISTERS_READ_BURST, count, values, updateParams, address);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::write(AddressType address, AddressType count, const ValueType values[])
{
    const auto increment = Registers<AddressType, ValueType>::m_increment;

    auto updateParams = [&address, &increment](decltype(count) wCount) {
        address += (wCount * increment);
    };

    m_commands.vendorWriteMulti(FN_REGISTERS_WRITE_BURST, count, values, updateParams, address);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::readBatch(const AddressType addresses[], AddressType count, ValueType values[])
{
    m_commands.vendorTransferMulti(FN_REGISTERS_BATCH, count, addresses, values);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::writeBatchImpl(const BatchType vals[], AddressType count)
{
    m_commands.vendorWriteMulti(FN_REGISTERS_BATCH, count, vals);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::setBits(AddressType address, ValueType bitmask)
{
    m_commands.vendorWriteParameters(FN_REGISTERS_SET_BITS, address, bitmask);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::clearBits(AddressType address, ValueType bitmask)
{
    m_commands.vendorWriteParameters(FN_REGISTERS_CLEAR_BITS, address, bitmask);
}

template <typename AddressType, typename ValueType>
void RemoteRegisters<AddressType, ValueType>::modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask)
{
    m_commands.vendorWriteParameters(FN_REGISTERS_MODIFY_BITS, address, clearBitmask, setBitmask);
}


/**
 * This avoids having to have the complete implementation in the header file,
 * since the linker of an external executable needs to find the implementation
 * of the class. It has to be after the member declaration, since some compilers
 * otherwise would not emit the symbols.
 */
template class RemoteRegisters<uint8_t>;
template class RemoteRegisters<uint8_t, uint16_t>;
template class RemoteRegisters<uint8_t, uint32_t>;
template class RemoteRegisters<uint16_t, uint32_t>;
template class RemoteRegisters<uint16_t>;
