/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/access/IMemory.hpp>


template <typename AddressType, typename ValueType = AddressType>
class Memory :
    public IMemory<AddressType, ValueType>
{
public:
    explicit Memory(AddressType increment = sizeof(ValueType));

    using typename IMemory<AddressType, ValueType>::BatchType;

    using IMemory<AddressType, ValueType>::read;
    using IMemory<AddressType, ValueType>::write;

    // the following functions should be overridden with optimized implementations for a certain physical interface if possible
    void read(AddressType address, AddressType count, ValueType values[]) override;
    void write(AddressType address, AddressType count, const ValueType values[]) override;

    void readBatch(const AddressType address[], AddressType count, ValueType values[]) override;
    void writeBatch(const BatchType vals[], AddressType count, bool optimize = false) override;

    // the following functions are typically not required to be overridden, but it can still make sense
    void read(AddressType address, ValueType &value) override;

    void setBits(AddressType address, ValueType bitmask) override;
    void clearBits(AddressType address, ValueType bitmask) override;
    void modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask) override;

protected:
    virtual void writeBatchImpl(const BatchType vals[], AddressType count);

    AddressType m_increment;
};

/**
 * This is for the linker of an external executable to find the implementation
 * of the class. Otherwise the complete implementation would have to be in the header file.
 */
extern template class Memory<uint8_t>;
extern template class Memory<uint8_t, uint16_t>;
extern template class Memory<uint8_t, uint32_t>;
extern template class Memory<uint16_t, uint32_t>;
extern template class Memory<uint16_t>;
extern template class Memory<uint32_t>;
extern template class Memory<uint32_t, uint8_t>;
