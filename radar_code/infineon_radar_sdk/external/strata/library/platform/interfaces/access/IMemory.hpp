/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstdint>
#include <type_traits>


///
/// \brief Interface for accessing a device's memory with a pre-defined value size
///
template <typename AddressType, typename ValueType = AddressType>
class IMemory
{
public:
    virtual ~IMemory() = default;

    ///
    /// \brief Reads the value at the given address
    /// \param address The address where to read
    /// \return The value read
    ///
    virtual ValueType read(AddressType address) = 0;

    ///
    /// \brief Reads the value at the given address
    /// \param address The address where to read
    /// \param value Reference parameter where the value will be written to
    ///
    virtual void read(AddressType address, ValueType &value) = 0;

    ///
    /// \brief Reads the values of consecutive memory starting from a given address
    /// \param address The address where to start reading
    /// \param count The number of values to read
    /// \param values An array where the read values will be written to
    ///
    virtual void read(AddressType address, AddressType count, ValueType values[]) = 0;

    /// \brief Writes address/value pairs given as a fixed-size array of BatchType
    /// \param vals An array with size N of address/value pairs to write
    /// \param optimize When true, vals will be written in an optimized way for improve performance
    ///        (e.g. ordered by the address and combined to bursts, depending on the implementation)
    ///        When false, no optimization is done, vals will be written as provided.
    ///
    template <std::size_t N>
    inline void write(AddressType address, const ValueType (&values)[N])
    {
        write(address, N, values);
    }

    ///
    /// \brief Writes a value at the given address
    /// \param address The address where to write
    /// \param value The value to write
    ///
    virtual void write(AddressType address, ValueType value) = 0;

    ///
    /// \brief Writes the values of consecutive memory starting from a given address
    /// \param address The address where to start writing
    /// \param count The number of values to write
    /// \param values An array of values to write
    ///
    virtual void write(AddressType address, AddressType count, const ValueType values[]) = 0;

    ///
    /// \brief POD type for storing address value pairs for writeBatch() calls
    ///
    struct BatchType
    {
        AddressType address;
        ValueType value;

        bool operator<(const BatchType &other) const
        {
            return this->address < other.address;
        }

        static constexpr std::size_t serialized_sizeof() noexcept
        {
            return sizeof(AddressType) + sizeof(ValueType);
        }
    };

    ///
    /// \brief Reads the values of non consecutive memory given an array of addresses
    /// \param address An Array of addresses to read from
    /// \param count The number of values to read
    /// \param values An array where the read values will be written to
    ///
    virtual void readBatch(const AddressType addresses[], AddressType count, ValueType values[]) = 0;

    ///
    /// \brief Writes multiple address/value pairs given as an array of BatchType
    /// \param vals An array of address/value pairs to write
    /// \param count The number of values to write
    /// \param optimize When true, vals will be written in an optimized way for improve performance
    ///        (e.g. ordered by the address and combined to bursts, depending on the implementation)
    ///        When false, no optimization is done, vals will be written as provided.
    ///
    virtual void writeBatch(const BatchType vals[], AddressType count, bool optimize = false) = 0;


    /// the following needs to be templated for correct overload resolution between literal zero and pointers

    template <typename T>
    void read(const T addresses[], AddressType count, ValueType values[])
    {
        readBatch(addresses, count, values);
    }

    template <typename T>
    void write(const T vals[], AddressType count, bool optimize = false)
    {
        writeBatch(vals, count, optimize);
    }

    ///
    /// \brief Writes multiple address/value pairs given as a 2D array of AddressType
    ///        (This is only available, when AddressType and ValueType are the same!)
    /// \param vals An array of address/value pairs to write
    /// \param count The number of values to write
    /// \param optimize When true, vals will be written in an optimized way for improve performance
    ///        (e.g. ordered by the address and combined to bursts, depending on the implementation)
    ///        When false, no optimization is done, vals will be written as provided.
    ///
    template <typename T = void>
    inline typename std::enable_if<std::is_same<AddressType, ValueType>::value, T>::type
    write(const AddressType vals[][2], AddressType count, bool optimize = false)
    {
        static_assert(sizeof(BatchType) == sizeof(*vals), "BatchType cannot be reinterpreted as an array of 2!");
        write(reinterpret_cast<const BatchType *>(vals), count, optimize);
    }

    /// \brief Writes address/value pairs given as a fixed-size array of BatchType
    /// \param vals An array with size N of address/value pairs to write
    /// \param optimize When true, vals will be written in an optimized way for improve performance
    ///        (e.g. ordered by the address and combined to bursts, depending on the implementation)
    ///        When false, no optimization is done, vals will be written as provided.
    ///
    template <std::size_t N>
    inline void write(const BatchType (&vals)[N], bool optimize = false)
    {
        write(vals, N, optimize);
    }

    ///
    /// \brief Sets single bits at the given address
    /// \param address The address of the value in memory to modify
    /// \param bitmask Value specifying which bits to set. 1 means set the bit, 0 means leave as is.
    ///
    virtual void setBits(AddressType address, ValueType bitmask) = 0;

    ///
    /// \brief Clears single bits at the given address
    /// \param address The address of the value in memory to modify
    /// \param bitmask Value specifying which bits to clear. 1 means clear the bit, 0 means leave as is.
    ///
    virtual void clearBits(AddressType address, ValueType bitmask) = 0;

    ///
    /// \brief Sets and clears single bits at the given address
    /// \param address The address of the value in memory to modify
    /// \param clearBitmask Value specifying which bits to clear. 1 means clear the bit, 0 means leave as is.
    /// \param setBitmask Value specifying which bits to set. 1 means set the bit, 0 means leave as is.
    ///
    virtual void modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask) = 0;
};
