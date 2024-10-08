/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBoard.hpp>

#include <algorithm>
#include <map>
#include <utility>


class Board :
    public IBoard
{
public:
    IComponent *getIComponent(uint16_t type, uint8_t id) override;
    IModule *getIModule(uint16_t type, uint8_t id) override;

    uint8_t getIModuleCount(uint16_t type) override;
    uint8_t getIComponentCount(uint16_t type) override;

protected:
    using KeyType = uint32_t;

    inline KeyType make_key(uint16_t type, uint8_t id)
    {
        return (type << 8) | id;
    }

    template <typename T>
    inline void registerInstance(T &instance, uint8_t id)
    {
        auto &map      = get_map(&instance);
        const auto key = make_key(T::getType(), id);
        map[key]       = &instance;
    }

    template <typename I>
    inline I *getInstance(uint16_t type, uint8_t id)
    {
        auto &map        = get_map(static_cast<I *>(0));
        const auto key   = make_key(type, id);
        const auto entry = map.find(key);
        if (entry != map.end())
        {
            return entry->second;
        }
        return nullptr;
    }

    template <typename I>
    inline uint8_t getCount(uint16_t type)
    {
        auto &map        = get_map(static_cast<I *>(0));
        using value_type = typename std::remove_reference<decltype(map)>::type::value_type;
        const auto count = std::count_if(map.begin(), map.end(), [type](const value_type &v) { return (v.first >> 8) == type; });
        return static_cast<uint8_t>(count);
    }

private:
    std::map<KeyType, IComponent *> m_components;
    std::map<KeyType, IModule *> m_modules;

    inline decltype(m_components) &get_map(IComponent *)
    {
        return m_components;
    }
    inline decltype(m_modules) &get_map(IModule *)
    {
        return m_modules;
    }
};
