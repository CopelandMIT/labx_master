/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "platform/interfaces/IBridge.hpp"
#include <memory>

class IBridgeData;
class IFrame;

///
/// \brief This class forms a base for implementations which wrap around IBridge interface to extend
/// the used bridge with specific functionality.
///
class BridgeWrapperBase : public IBridge
{
public:
    BridgeWrapperBase(const std::shared_ptr<IBridge> &bridge);

    ///
    /// \copydoc IBridge::isConnected
    ///
    bool isConnected() override;

    ///
    /// \copydoc IBridge::openConnection
    ///
    void openConnection() override;

    ///
    /// \copydoc IBridge::closeConnection
    ///
    void closeConnection() override;

    ///
    /// \copydoc IBridge::getIBridgeControl
    ///
    IBridgeControl *getIBridgeControl() override;

    ///
    /// \copydoc IBridge::getIBridgeData
    ///
    IBridgeData *getIBridgeData() override;

private:
    std::shared_ptr<IBridge> m_bridge;
};
