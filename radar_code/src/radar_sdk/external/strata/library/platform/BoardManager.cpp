/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BoardManager.hpp"

#include <common/Logger.hpp>
#include <common/exception/ENotImplemented.hpp>
#include <platform/BoardListProtocol.hpp>
#include <platform/exception/EAlreadyOpened.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EInUse.hpp>

#include <platform/ethernet/EnumeratorEthernet.hpp>
#include <serial/EnumeratorSerialImpl.hpp>
#include <uvc/EnumeratorUvcImpl.hpp>

#ifdef STRATA_CONNECTION_LIBUSB
    #include <platform/libusb/EnumeratorLibUsb.hpp>
#endif
#ifdef STRATA_CONNECTION_MCD
    #include <platform/wiggler/EnumeratorWiggler.hpp>
#endif

#include <common/cpp11/memory.hpp>
#include <cstring>

BoardManager::BoardManager() :
    m_selector {nullptr}
{
}

BoardManager::BoardManager(const char *interfaces) :
    m_selector {nullptr}
{
    parseConnectionTypes(interfaces, ',');
}

BoardManager::BoardManager(bool serial, bool ethernetUdp, bool uvc, bool wiggler, bool libusb) :
    m_selector {nullptr}
{
    LOG(WARN) << "This BoardManager constructor implementation is deprecated. Please don't use it anymore.";
    if (serial)
        addConnectionType(ConnectionType::serial);
    if (ethernetUdp)
        addConnectionType(ConnectionType::udp);
    if (uvc)
        addConnectionType(ConnectionType::uvc);
    if (libusb)
        addConnectionType(ConnectionType::libusb);
    if (wiggler)
        addConnectionType(ConnectionType::wiggler);
}

BoardManager::~BoardManager()
{
}

BoardManager &BoardManager::useSerial()
{
    addConnectionType(ConnectionType::serial);
    return *this;
}

BoardManager &BoardManager::useUdp()
{
    addConnectionType(ConnectionType::udp);
    return *this;
}

BoardManager &BoardManager::useTcp()
{
    addConnectionType(ConnectionType::tcp);
    return *this;
}

BoardManager &BoardManager::useUvc()
{
    addConnectionType(ConnectionType::uvc);
    return *this;
}

BoardManager &BoardManager::useLibusb()
{
    addConnectionType(ConnectionType::libusb);
    return *this;
}

BoardManager &BoardManager::useWiggler()
{
    addConnectionType(ConnectionType::wiggler);
    return *this;
}

BoardManager::ConnectionType BoardManager::getConnectionTypeByName(std::string name)
{
    // Remove leading and trailing whitespaces
    const char *spaces = " \t";
    auto start         = name.find_first_not_of(spaces);
    auto end           = name.find_last_not_of(spaces);
    if (start != std::string::npos)  //then also end is !=npos
    {
        // Only trim, if there is anything else than a space character
        name = name.substr(start, (end - start) + 1);
    }

    // Convert to lower case
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    if (name == "serial")
        return ConnectionType::serial;
    if (name == "udp")
        return ConnectionType::udp;
    if (name == "tcp")
        return ConnectionType::tcp;
    if (name == "uvc")
        return ConnectionType::uvc;
    if (name == "libusb")
        return ConnectionType::libusb;
    if (name == "wiggler")
        return ConnectionType::wiggler;
    return ConnectionType::unknown;
}

void BoardManager::addConnectionType(BoardManager::ConnectionType type)
{
    std::unique_ptr<IEnumerator> enumerator;
    switch (type)
    {
        case ConnectionType::serial:
            enumerator = std::make_unique<EnumeratorSerialImpl>();
            break;
        case ConnectionType::udp:
            if (m_enumerators.find(ConnectionType::tcp) != m_enumerators.end())
            {
                // A board supporting TCP always has to support UDP too for being found via broadcast (enumeration).
                // So it will always be recognized as UDP board if UDP is enabled. That's why only UDP OR TCP may be used,
                // not both at the same time.
                LOG(WARN) << "UDP and TCP cannot be used at the same time. UDP is ignored.";
            }
            else
            {
                enumerator = std::make_unique<EnumeratorEthernet>(false);
            }
            break;
        case ConnectionType::tcp:
            if (m_enumerators.find(ConnectionType::udp) != m_enumerators.end())
            {
                // A board supporting TCP always has to support UDP too for being found via broadcast (enumeration).
                // So it will always be recognized as UDP board if UDP is enabled. That's why only UDP OR TCP may be used,
                // not both at the same time.
                LOG(WARN) << "UDP and TCP cannot be used at the same time. TCP is ignored.";
            }
            else
            {
                enumerator = std::make_unique<EnumeratorEthernet>(true);
            }
            break;
        case ConnectionType::uvc:
            enumerator = std::make_unique<EnumeratorUvcImpl>();
            break;
        case ConnectionType::libusb:
#ifdef STRATA_CONNECTION_LIBUSB
            enumerator = std::make_unique<EnumeratorLibUsbImpl>();
#else
            LOG(WARN) << "LibUsb support was not enabled during build and will be ignored.";
#endif
            break;
        case ConnectionType::wiggler:
#ifdef STRATA_CONNETION_MCD
            enumerator = std::make_unique<EnumeratorWiggler>();
#else
            LOG(WARN) << "MCD support was not enabled during build. Wiggler will be ignored.";
#endif
            break;
        default:
            LOG(WARN) << "Unknown connection type will be ignored.";
            break;
    }
    if (enumerator)
    {
        m_enumerators[type] = std::move(enumerator);
    }
}

void BoardManager::parseConnectionTypes(const char *types, char separator)
{
    const std::string typesStr(types);
    size_t lastPos;
    size_t nextPos = -1;
    while (true)
    {
        lastPos = nextPos + 1;
        nextPos = typesStr.find(separator, lastPos);
        if (lastPos == typesStr.size())
        {
            // No more types
            return;
        }
        if (nextPos == std::string::npos)
        {
            //This is the last type
            addConnectionType(getConnectionTypeByName(typesStr.substr(lastPos)));
            return;
        }
        if (nextPos != lastPos)
        {
            // Only add non-empty types
            addConnectionType(getConnectionTypeByName(typesStr.substr(lastPos, nextPos - lastPos)));
        }
    }
}

void BoardManager::setEnumerationSelector(IEnumerationSelector *selector)
{
    m_selector = selector;
}

uint16_t BoardManager::enumerate(uint16_t maxCount)
{
    return enumerate(BoardListProtocol::begin, BoardListProtocol::end, maxCount);
}

uint16_t BoardManager::enumerate(BoardData::const_iterator begin, BoardData::const_iterator end, uint16_t maxCount)
{
    m_maxCount = maxCount;
    m_enumeratedList.clear();

    if (m_enumerators.empty())
    {
        LOG(WARN) << "No enumerators (connection types) selected. No boards will be found.";
    }

    for (auto &e : m_enumerators)
    {
        e.second->enumerate(*this, begin, end);
        if (m_maxCount && (m_enumeratedList.size() >= m_maxCount))
        {
            break;
        }
    }

    return static_cast<uint16_t>(m_enumeratedList.size());
}

BoardDescriptorList &BoardManager::getEnumeratedList()
{
    return m_enumeratedList;
}

std::unique_ptr<BoardInstance> BoardManager::createBoardInstance()
{
    if (m_enumeratedList.empty())
    {
        throw EConnection("No boards enumerated");
    }

    for (auto &d : m_enumeratedList)
    {
        LOG(DEBUG) << "Opening unused board ...";
        try
        {
            return d->createBoardInstance();
        }
        catch (const EAlreadyOpened &)
        {
            LOG(DEBUG) << "... board already used";
            continue;
        }
    }

    throw EConnection("... no unused board found");
}

std::unique_ptr<BoardInstance> BoardManager::createBoardInstance(uint8_t index)
{
    if (m_enumeratedList.empty())
    {
        throw EConnection("No boards enumerated");
    }
    if (index >= m_enumeratedList.size())
    {
        throw EConnection("Specified board index not found");
    }

    try
    {
        const auto &d = m_enumeratedList[index];
        return d->createBoardInstance();
    }
    catch (const EAlreadyOpened &)
    {
        throw EConnection("Specified board already in use");
    }
}

std::unique_ptr<BoardInstance> BoardManager::createBoardInstance(uint16_t vid, uint16_t pid)
{
    if (m_enumeratedList.empty())
    {
        throw EConnection("No boards enumerated");
    }

    for (auto &d : m_enumeratedList)
    {
        if ((d->getVid() == vid) && (d->getPid() == pid))
        {
            try
            {
                return d->createBoardInstance();
            }
            catch (const EAlreadyOpened &)
            {
                throw EConnection("Specified board already in use");
            }
        }
    }

    throw EConnection("Specified board not found");
}

std::unique_ptr<BoardInstance> BoardManager::createBoardInstance(const char name[])
{
    if (m_enumeratedList.empty())
    {
        throw EConnection("No boards enumerated");
    }

    for (auto &d : m_enumeratedList)
    {
        if (!strcmp(d->getName(), name))
        {
            try
            {
                return d->createBoardInstance();
            }
            catch (const EInUse &)
            {
                throw EConnection("Specified board already in use");
            }
        }
    }

    throw EConnection("Specified board not found");
}

STRATA_API std::unique_ptr<BoardInstance> BoardManager::createSpecificBoardInstance(const uint8_t uuid[UUID_LENGTH])
{
    if (m_enumeratedList.empty())
    {
        throw EConnection("No boards enumerated");
    }

    for (auto &d : m_enumeratedList)
    {
        LOG(DEBUG) << "Opening unused board ...";
        try
        {
            const auto boardUuid = d->getUuid();
            if (std::equal(boardUuid.begin(), boardUuid.end(), uuid))
            {
                return d->createBoardInstance();
            }
        }
        catch (const EAlreadyOpened &)
        {
            LOG(DEBUG) << "... board already used";
            continue;
        }
    }

    throw EConnection("Specified board not found");
}

bool BoardManager::onEnumerate(std::unique_ptr<BoardDescriptor> &&descriptor)
{
    bool select;
    if (m_selector)
    {
        select = m_selector->select(descriptor.get());
    }
    else
    {
        select = true;
    }

    if (select)
    {
        m_enumeratedList.push_back(std::move(descriptor));
    }

    return (m_maxCount && (m_enumeratedList.size() >= m_maxCount));
}
