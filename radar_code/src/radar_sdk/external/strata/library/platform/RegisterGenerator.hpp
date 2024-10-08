/**
 * @copyright 2022 Infineon Technologies
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <ostream>
#include <platform/NamedMemory.hpp>

template <typename AddressType, typename ValueType = AddressType>
class RegisterGenerator : private NamedMemory<AddressType, ValueType>
{
public:
    explicit RegisterGenerator(const char registerFile[]);
    explicit RegisterGenerator(const std::string &registerFile);

    void generateHeaderForC(std::ostream &stream, std::string prefix = std::string()) const;
    void generateHeaderForCpp(std::ostream &stream, std::string device_name) const;
    void generateRegisterMapForCpp(std::ostream &stream, std::string device_name) const;

protected:
    using NamedMemory<AddressType, ValueType>::m_layouts;
    using NamedMemory<AddressType, ValueType>::m_names;
};

/**
 * This is for the linker of an external executable to find the implementation
 * of the class. Otherwise the complete implementation would have to be in the header file.
 */
extern template class RegisterGenerator<uint8_t>;
extern template class RegisterGenerator<uint8_t, uint16_t>;
extern template class RegisterGenerator<uint8_t, uint32_t>;
extern template class RegisterGenerator<uint16_t, uint32_t>;
extern template class RegisterGenerator<uint16_t>;
extern template class RegisterGenerator<uint32_t>;
extern template class RegisterGenerator<uint32_t, uint8_t>;
