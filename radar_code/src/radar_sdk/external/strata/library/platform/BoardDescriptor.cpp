
#include "BoardDescriptor.hpp"

#include <platform/boards/BoardRemote.hpp>
#include <platform/exception/EConnection.hpp>


BoardDescriptor::BoardDescriptor(const BoardData &data, const char name[]) :
    m_data(data),
    m_name {name}
{}

BoardDescriptor::BoardDescriptor(const BoardData &data, const char name[], std::shared_ptr<IBridge> &&bridge) :
    m_data(data),
    m_name {name},
    m_bridge {std::move(bridge)}
{}

std::unique_ptr<BoardInstance> BoardDescriptor::createBoardInstance()
{
    checkBridge();
    if (m_bridge.use_count() > 1)
    {
        //throw EAlreadyOpened();
    }
    //return m_data.factory(std::move(m_bridge), this);
    auto bridge = m_bridge;
    return m_data.factory(std::move(bridge), this);
}

IBridge *BoardDescriptor::getIBridge()
{
    checkBridge();
    return m_bridge.get();
}

std::shared_ptr<IBridge> BoardDescriptor::createBridge()
{
    throw EConnection("BoardDescriptor does not contain any bridge");
}

void BoardDescriptor::checkBridge()
{
    if (!m_bridge)
    {
        m_bridge = createBridge();
    }
    if (!m_checked)
    {
        auto bridge = m_bridge.get();
        bridge->getIBridgeControl()->setDefaultTimeout();
        bridge->getIBridgeControl()->checkVersion();
        m_checked = true;
    }
}
