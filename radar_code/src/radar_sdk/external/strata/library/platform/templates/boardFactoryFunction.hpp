/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <platform/BoardDescriptor.hpp>

#include <common/cpp11/memory.hpp>

template <typename BoardType, int... args>
std::unique_ptr<BoardInstance> BoardFactoryFunction(std::shared_ptr<IBridge> &&bridge, BoardDescriptor *d)
{
    return std::make_unique<BoardInstance>(std::move(bridge), std::make_unique<BoardType>(bridge.get(), args...), d->getName(), d->getVid(), d->getPid());
}

template <typename BridgeType, typename BoardType, int... args>
std::unique_ptr<BoardInstance> BoardFactoryFunctionWrapped(std::shared_ptr<IBridge> &&bridge, BoardDescriptor *d)
{
    auto b = std::make_shared<BridgeType>(std::move(bridge));
    return std::make_unique<BoardInstance>(b, std::make_unique<BoardType>(b.get(), args...), d->getName(), d->getVid(), d->getPid());
}
