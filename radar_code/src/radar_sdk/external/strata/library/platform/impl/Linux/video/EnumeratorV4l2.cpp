/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "EnumeratorV4l2.hpp"

#include "BridgeV4l2.hpp"
#include <platform/BoardInstance.hpp>
#include <platform/BoardListProtocol.hpp>

#include <common/Logger.hpp>
#include <common/cpp11/memory.hpp>


bool BoardDescriptorV4l2::isUsed()
{
    return false;
}

std::shared_ptr<IBridge> BoardDescriptorV4l2::createBridge()
{
    return std::make_shared<BridgeV4l2>(std::move(m_devicePath));
}

EnumeratorV4l2::EnumeratorV4l2()
{
}

EnumeratorV4l2::~EnumeratorV4l2()
{
}

void EnumeratorV4l2::enumerate(IEnumerationListener &listener, BoardData::const_iterator begin, BoardData::const_iterator end)
{
    listener.onEnumerate(std::make_unique<BoardDescriptorV4l2>(*BoardListProtocol::begin, "test", "/dev/video0"));
}
