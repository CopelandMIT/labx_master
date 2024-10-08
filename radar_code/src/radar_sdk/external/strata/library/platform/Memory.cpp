/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "Memory.hpp"
#include <algorithm>
#include <common/Buffer.hpp>
#include <vector>


template <typename AddressType, typename ValueType>
Memory<AddressType, ValueType>::Memory(AddressType increment) :
    m_increment {increment}
{
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::read(AddressType address, ValueType &value)
{
    value = read(address);
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::read(AddressType address, AddressType count, ValueType values[])
{
    const auto last = values + count;
    while (values < last)
    {
        *values++ = read(address);
        address += m_increment;
    }
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::write(AddressType address, AddressType count, const ValueType values[])
{
    const auto last = values + count;
    while (values < last)
    {
        write(address, *values++);
        address += m_increment;
    }
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::readBatch(const AddressType address[], AddressType count, ValueType values[])
{
    const auto last = address + count;
    while (address < last)
    {
        *values++ = read(*address++);
    }
}


template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::writeBatch(const BatchType vals[], AddressType count, bool optimize)
{
    if (optimize)
    {
        std::vector<BatchType> sortedValues(vals, vals + count);
        std::sort(sortedValues.begin(), sortedValues.end());

        vals            = sortedValues.data();
        const auto last = vals + count;

        strata::buffer<ValueType> buffer(count);
        while (vals < last)
        {
            auto buf                = buffer.data();
            const auto addr         = vals->address;
            auto nextAddr           = addr;
            AddressType bufferCount = 0;
            do
            {
                *buf++ = vals++->value;
                bufferCount++;
                nextAddr += m_increment;
            } while ((vals != last) &&
                     (vals->address == nextAddr));

            write(addr, bufferCount, buffer.data());
        }
    }
    else
    {
        writeBatchImpl(vals, count);
    }
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::writeBatchImpl(const BatchType vals[], AddressType count)
{
    const auto last = vals + count;
    while (vals < last)
    {
        const auto val = vals++;
        write(val->address, val->value);
    }
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::setBits(AddressType address, ValueType bitmask)
{
    ValueType value;
    read(address, value);

    value |= bitmask;

    write(address, value);
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::clearBits(AddressType address, ValueType bitmask)
{
    ValueType value;
    read(address, value);

    value &= ~bitmask;

    write(address, value);
}

template <typename AddressType, typename ValueType>
void Memory<AddressType, ValueType>::modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask)
{
    ValueType value;
    read(address, value);

    value &= ~clearBitmask;
    value |= setBitmask;

    write(address, value);
}


/**
 * This avoids having to have the complete implementation in the header file,
 * since the linker of an external executable needs to find the implementation
 * of the class. It has to be after the member declaration, since some compilers
 * otherwise would not emit the symbols.
 */
template class Memory<uint8_t>;
template class Memory<uint8_t, uint16_t>;
template class Memory<uint8_t, uint32_t>;
template class Memory<uint16_t, uint32_t>;
template class Memory<uint16_t>;
template class Memory<uint32_t>;
template class Memory<uint32_t, uint8_t>;
