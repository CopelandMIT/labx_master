/**
 * @copyright 2022 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "RegisterGenerator.hpp"
#include <iomanip>
#include <set>

template <typename AddressType, typename ValueType>
RegisterGenerator<AddressType, ValueType>::RegisterGenerator(const char registerFile[]) :
    NamedMemory<AddressType, ValueType>(nullptr, registerFile)
{}

template <typename AddressType, typename ValueType>
RegisterGenerator<AddressType, ValueType>::RegisterGenerator(const std::string &registerFile) :
    NamedMemory<AddressType, ValueType>(nullptr, registerFile)
{}

static std::string s_cpp_preamble = R"(#include <cstdint>

namespace Infineon
{
    namespace Internal
    {

        template<typename ValueType, typename DerivedRegister>
        class Register
        {
        public:
            template<ValueType Offset, ValueType Mask>
            struct Bitfield {};

            explicit Register(ValueType& value)
                : m_value(value)
            {}

            inline Register& clear()
            {
                m_value = 0;
                return *this;
            }

            template<ValueType Offset, ValueType Mask>
            inline Register& set(Bitfield<Offset, Mask>, ValueType value)
            {
                m_value &= ~Mask;
                m_value |= (value << Offset) & Mask;
                return *this;
            }

            template<ValueType Offset, ValueType Mask>
            inline const Register& get(Bitfield<Offset, Mask>, ValueType& value) const
            {
                value = (m_value & Mask) >> Offset;
                return *this;
            }
            
            inline const ValueType& value() const
            {
                return m_value;
            }

        private:
            ValueType& m_value;
        };
    }
}

)";

template <typename Type>
std::string get_type_name();

template <>
std::string get_type_name<uint8_t>()
{
    return "uint8_t";
}

template <>
std::string get_type_name<uint16_t>()
{
    return "uint16_t";
}

template <>
std::string get_type_name<uint32_t>()
{
    return "uint32_t";
}

template <typename AddressType, typename ValueType>
void RegisterGenerator<AddressType, ValueType>::generateHeaderForC(std::ostream &stream, std::string prefix) const
{
    using namespace std;

    /*
     * For pretty formatting, the maximum register name length is searched.
     * Register names are sorted by address, because that's the order
     * registers typically appear in the data sheet. (A map is used for
     * sorting, because map entries are sorted by key.)
     */
    size_t max_name_length = 0;
    map<AddressType, string> sorted_registers;
    for (auto &register_entry : m_names)
    {
        max_name_length = max(max_name_length, register_entry.first.size());
        sorted_registers.insert(make_pair(register_entry.second.address,
                                          register_entry.first));
    }

    /*
     * For each register the mapping from name to address is exported as
     * preprocessor constant.
     */
    stream << "// Registers" << endl;
    for (auto &register_entry : sorted_registers)
    {
        AddressType address = register_entry.first;
        const string &name  = register_entry.second;

        stream << "#define " << prefix << "REG_"
               << setw(max_name_length) << setfill(' ') << left
               << name
               << " 0x"
               << setw(sizeof(AddressType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
               << hex << address << endl;
    }

    stream << "// Register Reset Values" << endl;
    for (auto &register_entry : sorted_registers)
    {
        const string &name        = register_entry.second;
        const auto &reset_value   = m_names.at(name).reset_value;
        const string reset_suffix = "_rst";

        stream << "#define " << prefix << "REG_"
               << setw(max_name_length + reset_suffix.size()) << setfill(' ') << left
               << name + reset_suffix
               << "   "
               << " 0x"
               << setw(sizeof(ValueType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
               << hex << reset_value << endl;
    }

    // Bit fields of each registers are exported, one block for each register.
    stream << "// Register Bitfields" << endl;
    for (auto &layout : m_layouts)
    {
        /*
         * For pretty formatting, the maximum bit field name length is
         * searched. At the end the maximum length is increased a little
         * because the exported names are appended by a suffix. Bit field
         * names are sorted by offset, because that's the order bit fields
         * typically appear in the data sheet. (A map is used for sorting,
         * because map entries are sorted by key.)
         */
        max_name_length = 0;
        map<ValueType, string> sorted_bitfields;
        for (auto &bitfield_entry : layout.bitfields)
        {
            max_name_length = max(max_name_length, bitfield_entry.first.size());
            sorted_bitfields.insert(make_pair(bitfield_entry.second.offset,
                                              bitfield_entry.first));
        }
        max_name_length += 4;

        /*
         * For each bit field the mapping from name to address is exported as
         * two preprocessor constants, one for the offset and one for a
         * register mask containing ones at all places covered by the bit
         * field.
         */
        stream << endl;
        for (auto &bitfield_entry : sorted_bitfields)
        {
            unsigned offset    = bitfield_entry.first;
            const string &name = bitfield_entry.second;
            ValueType mask     = layout.bitfields.at(name).mask;

            stream << "#define " << prefix << layout.name << "_"
                   << setw(max_name_length) << setfill(' ') << left
                   << name + "_pos"
                   << "   "
                   << setw(sizeof(ValueType) * 2) << setfill(' ') << right
                   << dec << offset << endl;
            stream << "#define " << prefix << layout.name << "_"
                   << setw(max_name_length) << setfill(' ') << left
                   << name + "_msk"
                   << " 0x"
                   << setw(sizeof(ValueType) * 2) << setfill('0') << right
                   << hex << mask << endl;
        }
    }
}

template <typename AddressType, typename ValueType>
void RegisterGenerator<AddressType, ValueType>::generateHeaderForCpp(std::ostream &stream, std::string device_name) const
{
    using namespace std;

    /*
     * First a preamble is written that declares base types for exported
     * registers and opens the namespace the exported registers are put into.
     */
    stream << s_cpp_preamble
           << "namespace Infineon" << endl
           << "{" << endl
           << "    namespace " << device_name << endl
           << "    {" << endl
           << "        template <typename DerivedRegister>" << endl
           << "        using Register = Infineon::Internal::Register<"
           << get_type_name<ValueType>() << ", DerivedRegister>;" << endl;

    /*
     * Register names are sorted by address, because that's the order
     * registers typically appear in the data sheet. (A map is used for
     * sorting, because map entries are sorted by key.)
     */
    map<AddressType, string> sorted_registers;
    for (auto &register_entry : m_names)
    {
        sorted_registers.insert(make_pair(register_entry.second.address,
                                          register_entry.first));
    }

    /*
     * This container remembers the register layouts that have already been
     * exported. This is used to avoid a layout again and again if it applies
     * to multiple registers.
     */
    std::set<std::string> exported_layouts;

    /*
     * For each register the mapping from name to address is exported as
     * preprocessor constant.
     */
    for (auto &register_entry : sorted_registers)
    {
        AddressType address         = register_entry.first;
        const string &register_name = register_entry.second;
        const auto &layout          = m_layouts.at(m_names.at(register_name).layout_index);
        const auto &reset_value     = m_names.at(register_name).reset_value;

        /*
         * For each register layout a class is generated that contains all bit
         * fields as nested classes. In most cases the register layout is just
         * used by one register and register name and layout name match. In
         * this case the exported layout class is already the class representing
         * the register.
         */
        if (exported_layouts.count(layout.name) == 0)
        {
            /*
             * At this point the address is only exported, if the register
             * layout applies only to this register. Otherwise it is exported
             * below in a derived class.
             */
            stream << endl
                   << "        struct " << layout.name
                   << " : public Register<" << layout.name << ">" << endl
                   << "        {" << endl;

            if (register_name == layout.name)
            {
                stream << "            enum" << endl
                       << "            {" << endl

                       << "                address = 0x"
                       << setw(sizeof(AddressType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
                       << hex << address << "," << endl

                       << "                reset_value = 0x"
                       << setw(sizeof(ValueType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
                       << hex << reset_value << endl

                       << "            };" << endl
                       << endl;
            }
            stream << "            using Register::Register;" << endl
                   << endl;

            /*
             * For pretty formatting, the maximum bit field name length is
             * searched. Bit field names are sorted by offset, because that's
             * the order bit fields typically appear in the data sheet. (A map
             * is used for sorting, because map entries are sorted by key.)
             */
            size_t max_name_length = 0;
            map<ValueType, string> sorted_bitfields;
            for (auto &bitfield_entry : layout.bitfields)
            {
                max_name_length = max(max_name_length,
                                      bitfield_entry.first.size());
                sorted_bitfields.insert(make_pair(bitfield_entry.second.offset,
                                                  bitfield_entry.first));
            }

            /*
             * For each bit field a nested class is added that carries the mask
             * and offset information.
             */
            for (auto &bitfield_entry : sorted_bitfields)
            {
                unsigned offset    = bitfield_entry.first;
                const string &name = bitfield_entry.second;
                ValueType mask     = layout.bitfields.at(name).mask;

                stream << "            using "
                       << setw(max_name_length) << setfill(' ') << left << name
                       << " = Bitfield<" << right
                       << setw(2) << setfill(' ') << dec << offset
                       << ", 0x" << setw(sizeof(ValueType) * 2) << setfill('0')
                       << hex << mask << ">;" << endl;
            }

            stream << "        };" << endl;
            exported_layouts.insert(layout.name);
        }

        /*
         * If the register shares its layout with other registers, another
         * class is exported for the register that derives from the common
         * layout class, and adds the register address.
         */
        if (register_name != layout.name)
        {
            stream << endl
                   << "        struct " << register_name
                   << " : public " << layout.name << endl
                   << "        {" << endl
                   << "            enum" << endl
                   << "            {" << endl

                   << "                address = 0x"
                   << setw(sizeof(AddressType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
                   << hex << address << "," << endl

                   << "                reset_value = 0x"
                   << setw(sizeof(ValueType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
                   << hex << reset_value << endl

                   << "            };" << endl
                   << endl

                   << "            using " << layout.name << "::"
                   << layout.name << ";" << endl
                   << "        };" << endl;
        }
    }

    // Finally namespaces are closed.
    stream << "    }" << endl
           << "}" << endl;
}

template <typename AddressType, typename ValueType>
void RegisterGenerator<AddressType, ValueType>::generateRegisterMapForCpp(std::ostream &stream, std::string device_name) const
{
    using namespace std;

    stream << "#include <platform/NamedMemory.hpp>" << endl
           << endl
           << "#include <map>" << endl
           << "#include <string>" << endl
           << endl
           << "namespace Infineon" << endl
           << "{" << endl
           << "namespace " << device_name << endl
           << "{" << endl;

    /*
     * Register names are sorted by address, because that's the order
     * registers typically appear in the data sheet. (A map is used for
     * sorting, because map entries are sorted by key.)
     */
    map<AddressType, string> sorted_registers;
    for (auto &register_entry : m_names)
    {
        sorted_registers.insert(make_pair(register_entry.second.address,
                                          register_entry.first));
    }

    stream << "// For auto-generated code clang format is skipped." << endl
           << "// clang-format off" << endl
           << endl
           << "// All registers with: name, address, reset value, layout index" << endl
           << "const std::map<std::string, NamedMemory<" << get_type_name<AddressType>() << ", " << get_type_name<ValueType>() << ">"
           << "::Address> registers {" << endl;
    for (auto &register_entry : sorted_registers)
    {
        AddressType address         = register_entry.first;
        const string &register_name = register_entry.second;
        const auto &reset_value     = m_names.at(register_name).reset_value;

        stream << "    {"
               << "\"" << register_name << "\""
               << ", {0x"
               << setw(sizeof(AddressType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
               << hex << address
               << ", 0x"
               << setw(sizeof(ValueType) * 2) << setfill('0') << right  // "* 2" two hex digits represent one byte
               << hex << reset_value
               << ", "
               << dec << 0  // layout index
               << "}}," << endl;
    }
    stream << "};" << endl
           << endl;

    stream << "// End of auto-generated code" << endl
           << "// clang-format on" << endl
           << endl;

    // Finally namespaces are closed.
    stream << "}" << endl
           << "}" << endl;
}

/**
 * This avoids having to have the complete implementation in the header file,
 * since the linker of an external executable needs to find the implementation
 * of the class. It has to be after the member declaration, since some compilers
 * otherwise would not emit the symbols.
 */
template class RegisterGenerator<uint8_t>;
template class RegisterGenerator<uint8_t, uint16_t>;
template class RegisterGenerator<uint8_t, uint32_t>;
template class RegisterGenerator<uint16_t, uint32_t>;
template class RegisterGenerator<uint16_t>;
template class RegisterGenerator<uint32_t>;
template class RegisterGenerator<uint32_t, uint8_t>;
template class RegisterGenerator<uint16_t, uint8_t>;
