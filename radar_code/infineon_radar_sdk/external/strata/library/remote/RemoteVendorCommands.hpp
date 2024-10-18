/**
 * @copyright 2020 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <platform/interfaces/IVendorCommands.hpp>
#include <universal/protocol/protocol_definitions.h>


class RemoteVendorCommands
{
public:
    RemoteVendorCommands(IVendorCommands *commands, uint8_t bRequest, uint16_t wType, uint8_t bId, uint8_t bSubInterface);

    template <typename T>
    inline void vendorWrite(uint8_t bFunction, T &&t)
    {
        m_commands->vendorWrite(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), sizeof(T), std::forward<T>(t));
    }

    template <typename... Args>
    void vendorWrite(uint8_t bFunction, Args &&...args)
    {
        m_commands->vendorWrite(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void vendorWriteParameters(uint8_t bFunction, Args &&...args)
    {
        m_commands->vendorWriteParameters(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), std::forward<Args>(args)...);
    }

    template <typename Return, typename Class, typename... Args>
    void vendorWriteParameters(uint8_t bFunction, Return (Class::*pmf)(Args...), Args &&...args)
    {
        m_commands->vendorWriteParameters(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), pmf, std::forward<Args>(args)...);
    }

    template <typename T>
    void vendorRead(uint8_t bFunction, T &t) const
    {
        m_commands->vendorRead(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), sizeof(T), &t);
    }

    template <typename T>
    void vendorRead(uint8_t bFunction, uint16_t wLength, T *buf) const
    {
        m_commands->vendorRead(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), wLength, buf);
    }

    template <typename... Args>
    void vendorReadParameters(uint8_t bFunction, Args &&...args) const
    {
        m_commands->vendorReadParameters(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), std::forward<Args>(args)...);
    }

    template <typename Return, typename Class, typename... Args>
    void vendorReadParameters(uint8_t bFunction, Return (Class::*pmf)(Args...), Args &&...args) const
    {
        m_commands->vendorReadParameters(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), pmf, std::forward<Args>(args)...);
    }

    template <typename TS, typename TR>
    void vendorTransfer(uint8_t bFunction, uint16_t wLengthSend, const TS bufferSend[], uint16_t &wLengthReceive, TR bufferReceive[])
    {
        m_commands->vendorTransfer(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), wLengthSend, bufferSend, wLengthReceive, bufferReceive);
    }

    // temporary overload to allow nullptr resolution... should be fixed by swapping order of send / receive parameters and using nullptr as default
    template <typename TR>
    void vendorTransfer(uint8_t bFunction, uint16_t, const std::nullptr_t, uint16_t &wLengthReceive, TR bufferReceive[])
    {
        m_commands->vendorTransfer(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), 0, nullptr, wLengthReceive, bufferReceive);
    }

    template <typename TS, typename TR>
    void vendorTransferChecked(uint8_t bFunction, uint16_t wLengthSend, const TS bufferSend[], uint16_t wLengthReceive, TR bufferReceive[])
    {
        m_commands->vendorTransferChecked(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), wLengthSend, bufferSend, wLengthReceive, bufferReceive);
    }


    /*
     * This function optimizes writing a buffer in multiple chunks, where also additional parameters,
     * e.g. a write address, need to be updated per chunk.
     * All data is accessed by reference, so updateFunction can modify args implicitly if necessary
     */
    template <typename T, typename C, typename L, typename... Args>
    void vendorWriteMulti(uint8_t bFunction, C count, const T buffer[], L updateFunction, Args &&...args)
    {
        decltype(count) wCount;
        auto payload = make_payloadBuffer(buffer, count, m_commands->getMaxTransfer(), std::forward<Args>(args)...);
        while ((wCount = payload.update()))
        {
            m_commands->vendorWrite(m_bRequest, CMD_W_VALUE(m_wType), CMD_W_INDEX(m_bId, m_bSubInterface, bFunction), payload.size(), payload.data());
            updateFunction(wCount);
            payload.fill(std::forward<Args>(args)...);
        }
    }

    /*
     * This function simply writes multiple chunks of a buffer without additional parameters
     */
    template <typename T, typename C>
    void vendorWriteMulti(uint8_t bFunction, C count, const T buffer[])
    {
        auto updateFunction = [](C) {};
        vendorWriteMulti(bFunction, count, buffer, updateFunction);
    }

    /*
     * This function writes multiple chunks of a buffer and reads chunks at the same time
     */
    template <typename S, typename R, typename C, typename L, typename... Args>
    void vendorTransferMulti(uint8_t bFunction, C count, const S bufferSend[], R bufferReceive[], L updateFunction, Args &&...args)
    {
        decltype(count) wCount;
        auto maxTransfer = m_commands->getMaxTransfer();
        if (sizeof(S) < sizeof(R))
        {
            // we need uint32_t for the first multiplication to avoid a uint16_t overflow
            maxTransfer = static_cast<uint16_t>(static_cast<uint32_t>(maxTransfer * sizeof(S) / sizeof(R)));
        }

        auto payload = make_payloadBuffer(bufferSend, count, maxTransfer, std::forward<Args>(args)...);
        while ((wCount = payload.update()))
        {
            const uint16_t wLengthReceive = wCount * sizeof(bufferReceive[0]);
            vendorTransferChecked(bFunction, payload.size(), payload.data(), wLengthReceive, bufferReceive);
            updateFunction(wCount);
            payload.fill(std::forward<Args>(args)...);
        }
    }

    template <typename S, typename R, typename C>
    void vendorTransferMulti(uint8_t bFunction, C count, const S bufferSend[], R bufferReceive[])
    {
        auto updateFunction = [](C) {};
        vendorTransferMulti(bFunction, count, bufferSend, bufferReceive, updateFunction);
    }

    /*
     * This function optimizes reading a buffer in multiple chunks, where also additional parameters,
     * e.g. a read address, need to be updated per chunk.
     * All data is accessed by reference, so updateFunction can modify args implicitly if necessary
     */
    template <typename T, typename C, typename L, typename... Args>
    void vendorTransferMultiRead(uint8_t bFunction, C count, T buffer[], L updateFunction, Args &&...args)
    {
        const uint16_t maxCount = m_commands->getMaxTransfer() / sizeof(T);
        decltype(count) wCount  = (maxCount < count) ? static_cast<C>(maxCount) : count;

        auto payload = make_payloadWriteParameters(wCount, std::forward<Args>(args)...);
        while (count > 0)
        {
            const uint16_t wLengthReceive = wCount * sizeof(buffer[0]);
            vendorTransferChecked(bFunction, payload.size(), payload.data(), wLengthReceive, buffer);

            buffer += wCount;
            count -= wCount;
            updateFunction(wCount);

            if (count < wCount)
            {
                // if we are at the last chunk, use only the remaining count
                wCount = count;
            }
            payload.fill(wCount, std::forward<Args>(args)...);
        }
    }

    /*
     * This function optimizes reading serial data from the communication interface to a struct
     * by not needing an additional buffer for receiving the data.
     * It directly reads the data to the end of the struct and then deserializes it
     * into the same memory.
     */
    template <typename T>
    void vendorReadToStruct(uint8_t bFunction, T *targetStruct, uint8_t count = 1)
    {
        constexpr size_t structSerialSize = serialized_sizeof(T());
        const uint16_t readSize           = static_cast<uint16_t>(structSerialSize * count);
        const uint16_t offset             = static_cast<uint16_t>((sizeof(T) * count) - readSize);
        uint8_t *buf                      = reinterpret_cast<uint8_t *>(targetStruct) + offset;
        vendorRead(bFunction, readSize, buf);
        const uint8_t *it = buf;
        for (uint8_t i = 0; i < count; i++)
        {
            it = serialToHost(it, &targetStruct[i]);
        }
    }

    inline uint32_t getProtocolVersion() const
    {
        return m_commands->getProtocolVersion();
    }

    inline uint16_t getMaxTransfer() const
    {
        return m_commands->getMaxTransfer();
    }

private:
    IVendorCommands *m_commands;
    uint8_t m_bRequest;
    uint16_t m_wType;
    uint8_t m_bId;
    uint8_t m_bSubInterface;
};
