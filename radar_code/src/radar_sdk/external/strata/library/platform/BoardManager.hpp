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
#include <platform/BoardInstance.hpp>
#include <platform/interfaces/IEnumerator.hpp>

#include <functional>
#include <map>
#include <memory>

#define UUID_LENGTH 16


class IEnumerationSelector
{
public:
    virtual ~IEnumerationSelector() = default;

    ///
    /// This can be implemented by a caller to filter found boards by additional criteria.
    /// If specified, it will be called within onEnumerate and
    /// only add the descriptor to the list when true is returned.
    ///
    virtual bool select(BoardDescriptor *descriptor) = 0;
};

/**
 * @brief Helper to create enumeration selectors that take a predicate function, e.g. a lambda
 */
class EnumerationSelectorHelper : public IEnumerationSelector
{
public:
    explicit EnumerationSelectorHelper(const std::function<bool(BoardDescriptor *)> &predicate) :
        m_predicate(predicate)
    {
    }

    bool select(BoardDescriptor *descriptor) override
    {
        return descriptor && m_predicate(descriptor);
    }

private:
    std::function<bool(BoardDescriptor *)> m_predicate;
};

/**
 * @brief Class to enumerate and create board instances
 */
class BoardManager :
    private IEnumerationListener
{
public:
    /**
     * @brief Create a BoardManager instance without defining the interfaces for enumerating boards
     *        Use the separate useXXX() functions to add interfaces
     */
    STRATA_API BoardManager();

    /**
     * @brief Create a BoardManager instance and define the interfaces for enumerating boards as string.
     * @param interfaces Defines which interfaces to use for enumerating / connecting boards
     *        The parameter is a comma-separated list of connection types, like 'udp,serial'
     *        The following connection types are supported
     *        - serial: Serial port
     *        - udp: Ethernet protocol UDP
     *        - tcp: Ehternet protocol TCP (for enumeration UDP is used)
     *        - uvc: USB Video Class
     *        - libusb: USB via libusb
     *        - wiggler: Connection via the wiggler
     */
    STRATA_API BoardManager(const char *interfaces);

    /**
     * @brief Create a BoardManager instance and define the interfaces for enumerating boards.
     * @note The parameters are all optional. See their description for the default values.
     * @param serial Defines whether boards on the serial ports shall be enumerated
     * @param ethernet Defines whether boards connected via ethernet shall be enumerated, Default True
     * @param uvc Defines whether USB Video Class boards shall be enumerated, Default True
     * @param wiggler Defines whether boards on the wiggler interface shall be enumerated, Default False
     */
    STRATA_API BoardManager(bool serial, bool ethernetUdp = true, bool uvc = false, bool wiggler = false, bool libusb = true);

    STRATA_API virtual ~BoardManager() override;

    /**
     * @brief Add the serial connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useSerial();

    /**
     * @brief Add the Ethernet UDP connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useUdp();

    /**
     * @brief Add the Ethernet TCP connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useTcp();

    /**
     * @brief Add the USB Video Class connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useUvc();

    /**
     * @brief Add the libusb connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useLibusb();

    /**
     * @brief Add the wiggler connection type to be used during board enumeration
     * @return A reference to the BoardManager object, so that the useXXX() calls can be chained
     */
    STRATA_API BoardManager &useWiggler();

    STRATA_API void setEnumerationSelector(IEnumerationSelector *selector);

    /**
     * @brief Enumerate (collect) all boards on the activated interfaces (see constructor)
     * @note The function used an internal list to identify the board type.
     * @param maxCount Maximum number of boards to enumerate. If more boards are connected, they are ignored.
     * @return The number of boards found on all active interfaces
     */
    STRATA_API uint16_t enumerate(uint16_t maxCount = 0);

    /**
     * @brief Enumerate (collect) all boards on the activated interfaces (see constructor)
     * @note This function additionally takes a list of BoardData items describing board types and providing the factory for creating the instances.
     * @begin An iterator pointing to the begin of the BoardData list
     * @end  An iterator pointing to the end of the BoardData list
     * @param maxCount Maximum number of boards to enumerate. If more boards are connected, they are ignored.
     * @return The number of boards found on all active interfaces
     */
    STRATA_API uint16_t enumerate(BoardData::const_iterator begin, BoardData::const_iterator end, uint16_t maxCount = 0);

    /**
     * @brief Get the list of all enumerated boards
     * @return The board list
     */
    STRATA_API BoardDescriptorList &getEnumeratedList();

    /**
     * @brief Get the first unused board
     * @return The board instance if there is an unused board, otherwise throws an exception
     */
    STRATA_API std::unique_ptr<BoardInstance> createBoardInstance();

    /**
     * @brief Get the board with the provided index
     * @param index The index of the board in the enumerated list
     * @return The board instance if the specified board was found and is unused, otherwise throws an exception
     */
    STRATA_API std::unique_ptr<BoardInstance> createBoardInstance(uint8_t index);

    /**
     * @brief Get the first board of a specific type
     * @param vid The vendor ID of the board to connect to
     * @param pid The product ID of the board to connect to
     * @return The board instance if an unused board of this type was found, otherwise throws an exception
     */
    STRATA_API std::unique_ptr<BoardInstance> createBoardInstance(uint16_t vid, uint16_t pid);

    /**
     * @brief Get the first board of a specific type
     * @param name The type name of the board to connect to
     * @return The board instance if an unused board of this type was found, otherwise throws an exception
     */
    STRATA_API std::unique_ptr<BoardInstance> createBoardInstance(const char name[]);

    /**
     * @brief Get the board identified by the provided UUID
     * @note The UUID identifies only one board instance, even if there are multiple boards of the same type.
     * @param uuid The ID of the board to connect to
     * @return The board instance if the specified board was found, otherwise throws an exception
     */
    STRATA_API std::unique_ptr<BoardInstance> createSpecificBoardInstance(const uint8_t uuid[UUID_LENGTH]);
    inline std::unique_ptr<BoardInstance> createSpecificBoardInstance(const IBridgeControl::Uuid_t &uuid)
    {
        return createSpecificBoardInstance(uuid.data());
    }

protected:
    ///
    /// List of all boards found during enumeration.
    /// This list can then be filtered by the createBoardInstance functions.
    ///
    BoardDescriptorList m_enumeratedList;

    enum class ConnectionType
    {
        unknown,
        serial,
        udp,
        tcp,
        uvc,
        libusb,
        wiggler
    };

    ///
    /// List of all instantiated enumerators
    ///
    std::map<ConnectionType, std::unique_ptr<IEnumerator>> m_enumerators;

private:
    ///
    /// This will be called by enumerators with descriptors of found board.
    ///
    bool onEnumerate(std::unique_ptr<BoardDescriptor> &&descriptor) override;
    IEnumerationSelector *m_selector;
    uint16_t m_maxCount;

    static ConnectionType getConnectionTypeByName(std::string name);
    void addConnectionType(ConnectionType type);
    void parseConnectionTypes(const char *types, char separator);
};
