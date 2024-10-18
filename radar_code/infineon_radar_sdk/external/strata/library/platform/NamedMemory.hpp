/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <Definitions.hpp>
#include <platform/exception/EMemory.hpp>
#include <platform/interfaces/access/IMemory.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>


template <typename AddressType, typename ValueType = AddressType>
class NamedMemory
{
public:
    using BatchType      = typename IMemory<AddressType, ValueType>::BatchType;
    using NamedBatchType = std::pair<std::string, ValueType>;

    struct Bitfield
    {
        ValueType mask;
        ValueType offset;
        //        std::string desc;
        //        std::string info;
    };
    using Bitfields = std::map<std::string, Bitfield>;

    struct Layout
    {
        std::string name;
        Bitfields bitfields;
    };
    using Layouts = std::vector<Layout>;

    struct Address
    {
        AddressType address;
        ValueType reset_value;
        size_t layout_index;
    };
    using Addresses = std::map<std::string, Address>;


    STRATA_API NamedMemory(IMemory<AddressType, ValueType> *memory, const char registerFile[]);
    STRATA_API NamedMemory(IMemory<AddressType, ValueType> *memory, const std::string &registerFile);
    STRATA_API NamedMemory(IMemory<AddressType, ValueType> *memory, const Addresses &registerMap, const Layouts &layouts);
    STRATA_API NamedMemory(IMemory<AddressType, ValueType> *memory, Addresses &&registerMap, Layouts &&layouts);
    STRATA_API NamedMemory(IMemory<AddressType, ValueType> *memory);


    ValueType read(AddressType address)
    {
        return m_memory->read(address);
    }

    ValueType read(const std::string &name)
    {
        ValueType value;
        read(name, value);
        return value;
    }

    void read(const std::string &name, ValueType &value)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);
        value               = m_memory->read(address);
        if (bitfield)
        {
            value &= bitfield->mask;
            value >>= bitfield->offset;
        }
    }

    void read(const std::vector<std::string> &names, ValueType values[])
    {
        for (const auto &name : names)
        {
            AddressType address;
            const auto bitfield = getAddress(name, address);
            *values             = read(address);
            if (bitfield)
            {
                *values &= bitfield->mask;
                *values >>= bitfield->offset;
            }
            values++;
        }
    }

    template <typename... Args>
    void read(const std::string &name, Args &&...args)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);
        if (bitfield)
        {
            throw EMemory("Bitfield access cannot be used for burst read");
        }

        m_memory->read(address, std::forward<Args>(args)...);
    }


    void write(const std::string &name, ValueType value)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);
        if (bitfield)
        {
            const ValueType clearMask = bitfield->mask;
            const ValueType setMask   = value << bitfield->offset;
            m_memory->modifyBits(address, clearMask, setMask);
        }
        else
        {
            m_memory->write(address, value);
        }
    }

    void write(const std::vector<NamedBatchType> &values, bool optimize = false)
    {
        std::vector<BatchType> chunk;
        for (auto &entry : values)
        {
            AddressType address;
            const auto bitfield = getAddress(entry.first, address);
            if (bitfield)
            {
                if (!chunk.empty())
                {
                    m_memory->writeBatch(chunk.data(), static_cast<AddressType>(chunk.size()), optimize);
                    chunk.clear();
                }
                const ValueType setMask = entry.second << bitfield->offset;
                m_memory->modifyBits(address, bitfield->mask, setMask);
            }
            else
            {
                chunk.emplace_back(BatchType {address, entry.second});
            }
        }
        if (!chunk.empty())
        {
            m_memory->writeBatch(chunk.data(), static_cast<AddressType>(chunk.size()), optimize);
        }
    }

    void setBits(const std::string &name, ValueType &bitmask)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);

        if (bitfield)
        {
            bitmask <<= bitfield->offset;
            bitmask &= bitfield->mask;
        }
        m_memory->setBits(address, bitmask);
    }

    inline void setBits(AddressType address, ValueType &bitmask)
    {
        m_memory->setBits(address, bitmask);
    }

    void clearBits(const std::string &name, ValueType &bitmask)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);

        if (bitfield)
        {
            bitmask <<= bitfield->offset;
            bitmask &= bitfield->mask;
        }
        m_memory->clearBits(address, bitmask);
    }

    inline void clearBits(AddressType address, ValueType &bitmask)
    {
        m_memory->clearBits(address, bitmask);
    }

    void modifyBits(const std::string &name, ValueType clearBitmask, ValueType setBitmask)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);

        if (bitfield)
        {
            clearBitmask <<= bitfield->offset;
            clearBitmask &= bitfield->mask;
            setBitmask <<= bitfield->offset;
            setBitmask &= bitfield->mask;
        }
        m_memory->modifyBits(address, clearBitmask, setBitmask);
    }

    inline void modifyBits(AddressType address, ValueType clearBitmask, ValueType setBitmask)
    {
        m_memory->modifyBits(address, clearBitmask, setBitmask);
    }

    template <typename... Args>
    void write(const std::string &name, Args &&...args)
    {
        AddressType address;
        const auto bitfield = getAddress(name, address);
        if (bitfield)
        {
            throw EMemory("Bitfield access cannot be used for burst write");
        }

        m_memory->write(address, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void read(T address, Args &&...args)
    {
        m_memory->read(address, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void write(T address, Args &&...args)
    {
        m_memory->write(address, std::forward<Args>(args)...);
    }

    template <typename ContainerType>
    void loadConfig(const std::string &filename, ContainerType &registerList)
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
        {
            throw EMemory("NamedMemory<>::loadConfig() - file open failed");
        }
        while (ifs.good())
        {
            std::string registerName;
            AddressType address;
            ValueType value;
            ifs >> registerName >> std::hex >> address >> std::hex >> value;
            if (ifs.fail())
            {
                if (registerName.empty())
                {
                    break;  // empty lines are allowed
                }
                throw EMemory("NamedMemory<>::loadConfig() - file read failed");
            }
            addEntry(address, value, registerList);
        }
        ifs.close();
    }

    template <typename ContainerType>
    void saveConfig(const std::string &filename, const ContainerType &registerList, const std::string &hexPrefix = std::string("0x"))
    {
        std::ofstream outfile;
        outfile.open(filename, std::ofstream::out);
        if (!outfile.is_open())
        {
            throw EMemory("NamedMemory<>::saveConfig() - file open failed");
        }

        for (auto &entry : registerList)
        {
            toString(outfile, getEntryAddress(entry), getEntryValue(entry), hexPrefix);
        }
        outfile.close();
    }

    STRATA_API IMemory<AddressType, ValueType> *getIMemory() const;

protected:
    STRATA_API void toString(std::ostream &stream, AddressType address, ValueType value, const std::string &hexPrefix);

    STRATA_API static void addEntry(AddressType address, ValueType value, std::vector<BatchType> &registerList);
    STRATA_API static void addEntry(AddressType address, ValueType value, std::map<AddressType, ValueType> &registerList);

    STRATA_API static AddressType getEntryAddress(const BatchType &registerPair);
    STRATA_API static AddressType getEntryAddress(const std::pair<AddressType, ValueType> &registerPair);

    STRATA_API static ValueType getEntryValue(const BatchType &registerPair);
    STRATA_API static ValueType getEntryValue(const std::pair<AddressType, ValueType> &registerPair);

private:
    ///
    /// \brief getAddress
    /// \param name string containing the name for the address (optionally with a '.' separated bitfield name) or an address
    /// \param address the numeric value for the address
    /// \return pointer to the bitfield (optional), or nullptr if no bitfield was specified
    ///
    const Bitfield *getAddress(const std::string &name, AddressType &address);

    const std::string &getName(AddressType address);

    void loadFile(const std::string &filename);
    void loadJson(const std::string &filename);
    void loadXml(const std::string &filename);

    IMemory<AddressType, ValueType> *m_memory;

protected:
    Addresses m_names;
    Layouts m_layouts;
};


/**
 * This is for the linker of an external executable to find the implementation
 * of the class. Otherwise the complete implementation would have to be in the header file.
 */
extern template class NamedMemory<uint8_t>;
extern template class NamedMemory<uint8_t, uint16_t>;
extern template class NamedMemory<uint8_t, uint32_t>;
extern template class NamedMemory<uint16_t>;
extern template class NamedMemory<uint16_t, uint8_t>;
extern template class NamedMemory<uint16_t, uint32_t>;
extern template class NamedMemory<uint32_t>;
extern template class NamedMemory<uint32_t, uint8_t>;
