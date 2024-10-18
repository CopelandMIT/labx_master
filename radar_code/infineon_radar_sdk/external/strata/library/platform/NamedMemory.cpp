/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "NamedMemory.hpp"
#include <cctype>
#include <functional>
#include <limits>
#include <pugixml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>


template <typename AddressType, typename ValueType>
NamedMemory<AddressType, ValueType>::NamedMemory(IMemory<AddressType, ValueType> *memory, const char registerFile[]) :
    m_memory {memory}
{
    loadFile(registerFile);
}

template <typename AddressType, typename ValueType>
NamedMemory<AddressType, ValueType>::NamedMemory(IMemory<AddressType, ValueType> *memory, const std::string &registerFile) :
    m_memory {memory}
{
    loadFile(registerFile.c_str());
}

template <typename AddressType, typename ValueType>
NamedMemory<AddressType, ValueType>::NamedMemory(IMemory<AddressType, ValueType> *memory, const Addresses &registerMap, const Layouts &layouts) :
    m_memory {memory},
    m_names {registerMap},
    m_layouts {layouts}
{}

template <typename AddressType, typename ValueType>
NamedMemory<AddressType, ValueType>::NamedMemory(IMemory<AddressType, ValueType> *memory, Addresses &&registerMap, Layouts &&layouts) :
    m_memory {memory},
    m_names {std::move(registerMap)},
    m_layouts {std::move(layouts)}
{}

template <typename AddressType, typename ValueType>
NamedMemory<AddressType, ValueType>::NamedMemory(IMemory<AddressType, ValueType> *memory) :
    m_memory {memory},
    m_names {}
{}

template <typename AddressType, typename ValueType>
IMemory<AddressType, ValueType> *NamedMemory<AddressType, ValueType>::getIMemory() const
{
    return m_memory;
}

template <typename AddressType, typename ValueType>
const typename NamedMemory<AddressType, ValueType>::Bitfield *NamedMemory<AddressType, ValueType>::getAddress(const std::string &name, AddressType &address)
{
    if (std::isdigit(name.front()))
    {
        // we have a number as string, so try to convert it
        auto str = name.c_str();
        int base = 10;
        if (name.front() == '0')
        {
            switch (name.at(1))
            {
                case 'b':
                case 'B':
                    str += 2;  // skip prefix at the beginning of the string
                    base = 2;
                    break;
                case 'x':
                case 'X':
                    str += 2;  // skip prefix at the beginning of the string
                    base = 16;
                    break;
                default:
                    break;
            }
        }

        char *end;
        const auto val        = strtoul(str, &end, base);
        const std::size_t pos = end - name.c_str();
        if (pos != name.size())
        {
            throw EMemory("NamedMemory<>::getAddress() - numeric string conversion failed");
        }
        if ((val < std::numeric_limits<AddressType>::min()) || (val > std::numeric_limits<AddressType>::max()))
        {
            throw EMemory("NamedMemory<>::getAddress() - numeric string out of address range");
        }
        address = static_cast<AddressType>(val);
        return nullptr;
    }

    const auto separator = name.find('.');
    if (separator != std::string::npos)
    {
        // we have a bitfield access
        const auto parentName   = name.substr(0, separator);
        const auto bitfieldName = name.substr(separator + 1);
        const auto &entry       = m_names.at(parentName);

        address = entry.address;
        return &m_layouts.at(entry.layout_index).bitfields.at(bitfieldName);
    }

    // we just have a simple name to look up
    address = m_names.at(name).address;
    return nullptr;
}

template <typename AddressType, typename ValueType>
const std::string &NamedMemory<AddressType, ValueType>::getName(AddressType address)
{
    static const std::string notFound = "";

    auto it = std::find_if(m_names.begin(), m_names.end(), [address](const std::pair<std::string, Address> &p) {
        return p.second.address == address;
    });
    if (it == m_names.end())
    {
        return notFound;
    }
    else
    {
        return it->first;
    }
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::loadFile(const std::string &filename)
{
    if (filename.substr(filename.size() - 4) == ".xml")
    {
        loadXml(filename);
    }
    else if (filename.substr(filename.size() - 5) == ".json")
    {
        loadJson(filename);
    }
    else
    {
        throw EMemory("NamedMemory<>::loadFile() - unknown file extension");
    }
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::loadJson(const std::string &filename)
{
    rapidjson::Document document;
    std::ifstream ifs(filename);

    if (!ifs.is_open())
    {
        throw EMemory("NamedMemory<>::loadJson() - file open failed");
    }

    rapidjson::IStreamWrapper isw(ifs);
    document.ParseStream(isw);

    if (document.HasParseError())
    {
        throw EMemory("JSON parsing failed");
    }

    if (!document.HasMember("units") && !document.HasMember("registers"))
    {
        throw EMemory("JSON member units or registers missing");
    }

    auto registers = document["units"][0]["registers"].GetArray();

    for (const auto &reg : registers)
    {
        Layout layout;

        const auto register_name = reg["name"].GetString();
        const auto address       = reg["address"].GetInt();
        const auto reset_val     = reg["reset"][0]["value"].GetInt();

        const auto bslices = reg["bslices"].GetArray();
        for (const auto &bslice : bslices)
        {
            const auto bslice_name = bslice["name"].GetString();
            const auto width       = bslice["width"].GetInt();
            const auto offset      = bslice["offset"].GetInt();
            const auto mask        = ((1 << width) - 1) << offset;
            const auto reserved    = strstr("RSVD", bslice_name);
            if (!reserved)
            {
                layout.bitfields.insert({bslice_name, {static_cast<ValueType>(mask), static_cast<ValueType>(offset)}});
            }
        }

        layout.name = register_name;
        m_layouts.push_back(std::move(layout));
        m_names.insert({register_name, {static_cast<AddressType>(address), static_cast<ValueType>(reset_val), m_layouts.size() - 1}});
    }
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::loadXml(const std::string &filename)
{
    // The complete XML file is loaded and parsed using pugixml library.
    pugi::xml_document ecb_file;
    ecb_file.load_file(filename.c_str());
    auto component = ecb_file.child("Component");

    /*
     * Smartar ECB files use variables and constants that are stored in two
     * XML sub-trees. All variables and constants declarations are gathered
     * from these sub-trees. Afterwards the declaration blocks are no longer
     * needed and can be removed.
     */
    std::map<std::string, std::string> substitutions;
    auto remember_constant = [&substitutions](const pugi::xml_node &decl) {
        auto name           = decl.child("Name").child_value();
        auto value          = decl.child("DefaultValue").child_value();
        substitutions[name] = value;
    };

    auto declaration_block = component.child("GenericDeclBlock");
    for (const auto &declaration : declaration_block.children("GenericDecl"))
        remember_constant(declaration);
    ecb_file.remove_child(declaration_block);

    declaration_block = component.child("ParamDeclBlock");
    for (const auto &declaration : declaration_block.children("ParamDecl"))
        remember_constant(declaration);
    component.remove_child(declaration_block);

    /*
     * Text substitution is easier to perform on a string, so the XML tree is
     * dumped to a string in which each occurrence of a variable or constant
     * reference (e.g. ${x} or $x) is replaced by the according value.
     */
    std::stringstream xml_stream;
    ecb_file.save(xml_stream);
    auto xml_string = xml_stream.str();

    auto replace = [&xml_string](const std::string &what, const std::string &with) {
        std::string::size_type pos {};
        while (std::string::npos != (pos = xml_string.find(what, pos)))
        {
            xml_string.replace(pos, what.length(), with);
            pos += with.length();
        }
    };
    for (const auto &substitution : substitutions)
    {
        replace("$" + substitution.first, substitution.second);
        replace("${" + substitution.first + "}", substitution.second);
    }

    // The final XML string is parsed again to generate a new XML tree.
    ecb_file.load_string(xml_string.c_str());
    component = ecb_file.child("Component");

    /*
     * Information in XML nodes are available as string data only. Sometimes
     * numbers have to be parsed. The string representation may be hexadecimal.
     */
    auto get_number = [](const pugi::xml_node &node) -> unsigned {
        std::string text = node.child_value();
        if (text.substr(0, 2) == "0x")
            return std::stoul(text.substr(2), nullptr, 16);
        else
            return std::stoul(text);
    };

    /*
     * This function processes all nodes in the specified XML node range and
     * extracts bit field information from them. Bit fields may appear grouped
     * in BitFieldSequence elements which itself may appear inside of
     * BitFieldSequence elements, so this function does its job recursively.
     * The names of nested BitFieldSequence elements are concatenated and
     * parsed as prefix when this function calls itself to parse another
     * sequence. Sequences may be looped to repeat a bit field structure
     * several times. In this case a name suffix may be used to add the
     * iteration index to a bit field name.
     * This function returns a bit field offset, pointing after the last
     * bit field found. In case of sequence iterations this is used to
     * calculate the start offset of repeated sequences.
     */
    using node_range = pugi::xml_object_range<pugi::xml_named_node_iterator>;
    std::function<AddressType(Bitfields &, const node_range &, unsigned,
                              const std::string &, const std::string &)>
        gather_bit_fields;
    gather_bit_fields = [&](Bitfields &bitfields, const node_range &nodes,
                            unsigned start_offset, const std::string &prefix,
                            const std::string &suffix) -> unsigned {
        /*
         * Whenever a bit field is found, next_offset is updated to point after
         * that bit field.
         */
        AddressType next_offset = 0;

        for (const auto &node : nodes)
        {
            /*
             * Each node should contain a name and an offset relative to the
             * start offset of the parent sequence.
             */
            std::string type = node.attribute("xsi:type").as_string();
            std::string name = node.child("Name").child_value();
            unsigned offset  = get_number(node.child("Offset"));

            if (type == "BitFieldSequence")
            {
                /*
                 * Sequences contain also information how often that sequence
                 * is repeated.
                 */
                unsigned min_index = get_number(node.child("MinIndex"));
                unsigned count     = get_number(node.child("Count"));

                if (count > 0)
                {
                    /*
                     * If the count is non-zero the sequence is repeated. In
                     * this case this function calls itself for each loop
                     * repetition. Typically the name of the sequence followed
                     * by the iteration index as appended to the name prefix
                     * when the children of the sequence are processed.
                     * However, there is an exception: When the name starts
                     * with an underscores this means the name is hidden, so it
                     * is not appended to the prefix. In this case also the
                     * iteration index is not appended directly to the prefix
                     * but used as a suffix to the children's names.
                     * The offset found in the sequence's XML element is only
                     * valid for the first iteration. The next iterations
                     * always start after the last bit field in the previous
                     * iteration.
                     */
                    for (unsigned idx = min_index; idx < min_index + count; ++idx)
                    {
                        std::string new_prefix = prefix;
                        std::string new_suffix = suffix + std::to_string(idx);
                        if (name[0] != '_')
                        {
                            new_prefix += name + new_suffix + "_";
                            new_suffix.clear();
                        }

                        offset += gather_bit_fields(bitfields,
                                                    node.children("BitFieldSequenceElement"),
                                                    start_offset + offset,
                                                    new_prefix, new_suffix);
                    }
                }
                else
                {
                    /*
                     * If the count is zero, the sequence is not repeated but
                     * appears only once. The difference between count=0 and
                     * count=1 is, that there is no iteration index appended
                     * for count=0 (see above).
                     * This function just calls itself once to process the
                     * children of the sequence. The sequence name is appended
                     * to the name prefix, if does not start with an underscore
                     * which means "invisible" in ECB.
                     */
                    std::string new_prefix = prefix;
                    std::string new_suffix = suffix;
                    if (name[0] != '_')
                    {
                        new_prefix += name + new_suffix + "_";
                        new_suffix.clear();
                    }

                    offset += gather_bit_fields(bitfields,
                                                node.children("SequenceElement"),
                                                start_offset + offset,
                                                new_prefix, new_suffix);
                }
            }
            else if (type == "BitField")
            {
                /*
                 * When a bit field is found, it is added to the provided map
                 * of bit fields.
                 */
                unsigned width = get_number(node.child("Width"));

                Bitfield bitfield;
                bitfield.offset = start_offset + offset;
                bitfield.mask   = ((1 << width) - 1) << bitfield.offset;

                bitfields[prefix + name + suffix] = bitfield;

                /*
                 * The bit position right after the bit field is calculated and
                 * remembered for the case that the parent sequence is
                 * repeated.
                 */
                offset += width;
            }
            else if (type == "BitFieldGapElement")
            {
                /*
                 * Gap elements are placeholders for bit fields. They don't
                 * appear in the bit field list, but increase the offset
                 * counter. This is important for sequence repetitions to
                 * leave some room between the repeated bit fields.
                 */
                unsigned width = get_number(node.child("Width"));
                offset += width;
            }
            next_offset = offset;
        }
        return next_offset;
    };

    /*
     * This function processes all nodes in the specified XML node range and
     * extracts register information from them. Registers may appear grouped in
     * RegMemSequence elements which itself may appear inside of RegMemSequence
     * elements, so this function does its job recursively. The names of nested
     * RegMemSequence elements are concatenated and parsed as prefix when this
     * function calls itself to parse another sequence. Sequences may be looped
     * to repeat a register structure several times. In this case a name suffix
     * may be used to add the iteration index to a register name.
     * This function returns an address offset, pointing after the last
     * register found. In case of sequence iterations this is used to calculate
     * the start offset of repeated sequences.
     */
    uint32_t data_unit = 0;
    std::map<std::string, size_t> bitfield_indexes;
    std::function<AddressType(const node_range &, AddressType,
                              const std::string &, const std::string &,
                              const std::string &)>
        gather_registes;
    gather_registes = [&](const node_range &nodes, AddressType start_address,
                          const std::string &layout_prefix,
                          const std::string &reg_prefix,
                          const std::string &reg_suffix) -> AddressType {
        /*
         * Whenever a register is found, next_offset is updated to point after
         * that register.
         */
        AddressType next_offset = 0;

        for (const auto &node : nodes)
        {
            /*
             * Each node should contain a name and an address offset relative
             * to the start address of the parent sequence.
             */
            std::string type = node.attribute("xsi:type").as_string();
            std::string name = node.child("Name").child_value();
            unsigned offset  = get_number(node.child("Offset"));

            if (type == "RegMemSequence")
            {
                /*
                 * Sequences contain also information how often that sequence
                 * is repeated.
                 */
                unsigned min_index = get_number(node.child("MinIndex"));
                unsigned count     = get_number(node.child("Count"));

                if (count > 0)
                {
                    /*
                     * If the count is non-zero the sequence is repeated. In
                     * this case this function calls itself for each loop
                     * repetition. Typically the name of the sequence followed
                     * by the iteration index as appended to the name prefix
                     * when the children of the sequence are processed.
                     * However, there is an exception: When the name starts
                     * with an underscores this means the name is hidden, so it
                     * is not appended to the prefix. In this case also the
                     * iteration index is not appended directly to the prefix
                     * but used as a suffix to the children's names.
                     * The offset found in the sequence's XML element is only
                     * valid for the first iteration. The next iterations
                     * always start after the last register in the previous
                     * iteration.
                     */
                    for (unsigned idx = min_index; idx < min_index + count; ++idx)
                    {
                        std::string new_layout_prefix = layout_prefix;
                        std::string new_reg_prefix    = reg_prefix;
                        std::string new_reg_suffix    = reg_suffix + std::to_string(idx);
                        if (name[0] != '_')
                        {
                            new_layout_prefix += name + "_";
                            new_reg_prefix += name + new_reg_suffix + "_";
                            new_reg_suffix.clear();
                        }

                        offset += gather_registes(node.children("SequenceElement"),
                                                  start_address + offset,
                                                  new_layout_prefix,
                                                  new_reg_prefix, new_reg_suffix);
                    }
                }
                else
                {
                    /*
                     * If the count is zero, the sequence is not repeated but
                     * appears only once. The difference between count=0 and
                     * count=1 is, that there is no iteration index appended
                     * for count=0 (see above).
                     * This function just calls itself once to process the
                     * children of the sequence. The sequence name is appended
                     * to the name prefix, if does not start with an underscore
                     * which means "invisible" in ECB.
                     */
                    std::string new_layout_prefix = layout_prefix;
                    std::string new_reg_prefix    = reg_prefix;
                    std::string new_reg_suffix    = reg_suffix;
                    if (name[0] != '_')
                    {
                        new_layout_prefix += name + "_";
                        new_reg_prefix += name + new_reg_suffix + "_";
                        new_reg_suffix.clear();
                    }

                    offset += gather_registes(node.children("SequenceElement"),
                                              start_address + offset,
                                              new_layout_prefix,
                                              new_reg_prefix, new_reg_suffix);
                }
            }
            else if (type == "Register")
            {
                std::string register_name = reg_prefix + name + reg_suffix;
                std::string layout_name   = layout_prefix + name;

                /*
                 * When a register is found, it is added to the map of
                 * addresses, and the bit fields of the register are processed.
                 */
                Address register_info;
                register_info.address     = start_address + offset;
                register_info.reset_value = get_number(node.child("ResetType")
                                                           .child("ResetValue"));

                if (bitfield_indexes.count(layout_name) == 0)
                {
                    Layout layout;
                    layout.name = layout_name;
                    gather_bit_fields(layout.bitfields,
                                      node.children("BitFieldElement"),
                                      0, std::string(), std::string());

                    bitfield_indexes.insert({layout_name, m_layouts.size()});
                    m_layouts.push_back(std::move(layout));
                }

                register_info.layout_index = bitfield_indexes.at(layout_name);
                m_names.insert({register_name, register_info});

                /*
                 * The offset right after the register is calculated and
                 * remembered for the case that the parent sequence is
                 * repeated.
                 */
                unsigned data_width = get_number(node.child("DataWidth"));
                offset += data_width / data_unit;
            }
            else if (type == "GapElement")
            {
                /*
                 * Gap elements are placeholders for register. They don't
                 * appear in the register list, but increase the offset
                 * counter. This is important for sequence repetitions to
                 * leave some room between the repeated registers.
                 */
                unsigned data_width = get_number(node.child("DataWidth"));
                offset += data_width / data_unit;
            }
            next_offset = offset;
        }
        return next_offset;
    };

    /*
     * On top level all register and bit field information is stored in one or
     * more sub-trees with tag "RegMemSet". Each RegMemSet tree contains one or
     * more sub-trees with tag "RegMemElement" which represent either a single
     * register or a sequence of registers. Other children of the RegMemSet
     * tree are not relevant, even the information in tag "Name" is ignored.
     * The only important information about a RegMemSet is the start address,
     * but that is not stored in the RegMemSet sub-tree but in a separate
     * sub-tree on root layer with tag "Interface". The ID of each RegMemSet
     * tree is needed to look up the start address in the Interface tree.
     */
    auto address_table = component.child("Interface").child("AddressBlock");

    for (const auto &reg_mem_set : component.children("RegMemSet"))
    {
        // Start address of RegMemSet is looked up from XML tree.
        std::string name = reg_mem_set.child("Name").child_value();
        std::string id   = reg_mem_set.child("ID").child_value();
        data_unit        = get_number(reg_mem_set.child("DataUnit"));

        std::string query         = "./XRefRegMemSet[XRefTargetID=" + id + "]";
        auto table_entry          = address_table.select_node(query.c_str()).node();
        AddressType start_address = get_number(table_entry.child("Offset"));

        gather_registes(reg_mem_set.children("RegMemElement"), start_address,
                        std::string(), std::string(), std::string());
    }
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::toString(std::ostream &stream, AddressType address, ValueType value, const std::string &hexPrefix)
{
    const auto &registerName = getName(address);
    if (registerName.empty())
    {
        stream << "reg";
    }
    else
    {
        stream << registerName;
    }
    stream << std::hex << std::setfill('0');
    stream << " " << hexPrefix << std::setw(sizeof(AddressType) * 2) << address << " " << hexPrefix << std::setw(sizeof(ValueType) * 2) << value << '\n';
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::addEntry(AddressType address, ValueType value, std::vector<BatchType> &registerList)
{
    BatchType registerEntry;
    registerEntry.address = address;
    registerEntry.value   = value;
    registerList.emplace_back(registerEntry);
}

template <typename AddressType, typename ValueType>
void NamedMemory<AddressType, ValueType>::addEntry(AddressType address, ValueType value, std::map<AddressType, ValueType> &registerList)
{
    std::pair<AddressType, ValueType> registerEntry;
    registerEntry.first  = address;
    registerEntry.second = value;
    registerList.insert(registerEntry);
}

template <typename AddressType, typename ValueType>
AddressType NamedMemory<AddressType, ValueType>::getEntryAddress(const BatchType &registerPair)
{
    return registerPair.address;
}

template <typename AddressType, typename ValueType>
AddressType NamedMemory<AddressType, ValueType>::getEntryAddress(const std::pair<AddressType, ValueType> &registerPair)
{
    return registerPair.first;
}

template <typename AddressType, typename ValueType>
ValueType NamedMemory<AddressType, ValueType>::getEntryValue(const BatchType &registerPair)
{
    return registerPair.value;
}

template <typename AddressType, typename ValueType>
ValueType NamedMemory<AddressType, ValueType>::getEntryValue(const std::pair<AddressType, ValueType> &registerPair)
{
    return registerPair.second;
}


/**
 * This avoids having to have the complete implementation in the header file,
 * since the linker of an external executable needs to find the implementation
 * of the class. It has to be after the member declaration, since some compilers
 * otherwise would not emit the symbols.
 */
template class NamedMemory<uint8_t>;
template class NamedMemory<uint8_t, uint16_t>;
template class NamedMemory<uint8_t, uint32_t>;
template class NamedMemory<uint16_t>;
template class NamedMemory<uint16_t, uint8_t>;
template class NamedMemory<uint16_t, uint32_t>;
template class NamedMemory<uint32_t>;
template class NamedMemory<uint32_t, uint8_t>;
