#include "BridgeWrapperBase.hpp"

BridgeWrapperBase::BridgeWrapperBase(const std::shared_ptr<IBridge> &bridge) :
    m_bridge(bridge)
{
}

bool BridgeWrapperBase::isConnected()
{
    return m_bridge->isConnected();
}

void BridgeWrapperBase::openConnection()
{
    m_bridge->openConnection();
}

void BridgeWrapperBase::closeConnection()
{
    m_bridge->closeConnection();
}

IBridgeControl *BridgeWrapperBase::getIBridgeControl()
{
    return m_bridge->getIBridgeControl();
}

IBridgeData *BridgeWrapperBase::getIBridgeData()
{
    return m_bridge->getIBridgeData();
}
