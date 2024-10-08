/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/interfaces/IBridgeControl.hpp>
#include <platform/interfaces/IBridgeData.hpp>


/// A bridge implementation can derive from this interface to allow access to its implementation (or a member)
/// Using a template depending on the implementation type avoids requiring the implementation type in
/// the general interface
template <typename InterfaceType>
class IBridgeSpecificInterface
{
    friend class IBridge;

public:
    virtual ~IBridgeSpecificInterface() = default;

protected:
    /**
     * @brief conversion operator exposing the ImplType interface
     */
    virtual InterfaceType *getInterfaceImpl() = 0;
};


class IBridge
{
public:
    virtual ~IBridge() = default;

    virtual bool isConnected()     = 0;
    virtual void openConnection()  = 0;
    virtual void closeConnection() = 0;

    virtual IBridgeControl *getIBridgeControl() = 0;
    virtual IBridgeData *getIBridgeData()       = 0;


    /**
    * @brief provides an interface to ImplType if implemented and exposed by a specific bridge implementation.
    * @return pointer to the interface, nullptr if type doesn't exist or is not exposed
    */
    template <typename InterfaceType>
    inline InterfaceType *getSpecificInterface()
    {
        auto bridge = dynamic_cast<IBridgeSpecificInterface<InterfaceType> *>(this);
        if (bridge == nullptr)
        {
            return nullptr;
        }
        return bridge->getInterfaceImpl();
    }
};
