/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include <common/EndianConversion.hpp>
#include <common/Serialization.hpp>
#include <common/serialization/PayloadBuffer.hpp>
#include <platform/exception/EProtocol.hpp>

#include <stdint.h>
#include <type_traits>


class IVendorCommands
{
private:
    template <typename T>
    struct convert_endianness
    {
        constexpr static bool value = !(is_little_endian<T>::value || std::is_same<T, uint8_t>::value || std::is_same<T, int8_t>::value);
    };

public:
    virtual ~IVendorCommands() = default;

    virtual uint32_t getProtocolVersion() const = 0;
    virtual void setDefaultTimeout()            = 0;
    virtual uint16_t getMaxTransfer() const     = 0;

    virtual void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, const uint8_t buffer[])                                                               = 0;
    virtual void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t buffer[])                                                                      = 0;
    virtual void vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const uint8_t bufferSend[], uint16_t &wLengthReceive, uint8_t bufferReceive[]) = 0;


    void vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
    {
        vendorWrite(bRequest, wValue, wIndex, 0, static_cast<const uint8_t *>(nullptr));
    }

    template <typename S, typename T>
    typename std::enable_if<!convert_endianness<T>::value>::type
    vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, S wLength, const T buffer[])
    {
        vendorWrite(bRequest, wValue, wIndex, static_cast<uint16_t>(wLength), reinterpret_cast<const uint8_t *>(buffer));
    }

    template <typename S, typename T>
    typename std::enable_if<convert_endianness<T>::value>::type
    vendorWrite(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, S wLength, const T buffer[])
    {
        const auto count = wLength / sizeof(T);
        auto payload     = make_payloadBuffer(buffer, count);
        payload.update();
        vendorWrite(bRequest, wValue, wIndex, payload.size(), payload.data());
    }

    template <typename... Args>
    void vendorWriteParameters(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, Args &&...args)
    {
        auto payload = make_payloadWriteParameters(std::forward<Args>(args)...);
        vendorWrite(bRequest, wValue, wIndex, payload.size(), payload.data());
    }

    template <typename T>
    void vendorRead(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength, T buffer[])
    {
        vendorRead(bRequest, wValue, wIndex, wLength, reinterpret_cast<uint8_t *>(buffer));

        // the following should be optimized away by the compiler if target architecture is little endian
        const auto count = wLength / sizeof(T);
        littleToHost(buffer, buffer + count);
    }

    template <typename... Args>
    void vendorReadParameters(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, Args &...args)
    {
        auto payload = make_PayloadReadParameters(args...);
        vendorRead(bRequest, wValue, wIndex, payload.size(), payload.data());
        payload.unserialize(args...);
    }


    // The transfer function is used to write and read data with one single transaction.
    // There are 2 variants:
    // - Variable: wLengthReceive specifies the maximum length to be received and returnes the length actually received.
    // - Fixed: wLengthReceive specifies the exact length to receive. If a different length is received, it throws an error.


    template <typename TS, typename TR>
    typename std::enable_if<!convert_endianness<TS>::value>::type
    vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const TS bufferSend[], uint16_t &wLengthReceive, TR bufferReceive[])
    {
        vendorTransfer(bRequest, wValue, wIndex, wLengthSend, reinterpret_cast<const uint8_t *>(bufferSend), wLengthReceive, reinterpret_cast<uint8_t *>(bufferReceive));

        // the following should be optimized away by the compiler if target architecture is little endian
        const auto countReceive = wLengthReceive / sizeof(TR);
        littleToHost(bufferReceive, bufferReceive + countReceive);
    }

    template <typename TS, typename TR>
    typename std::enable_if<convert_endianness<TS>::value>::type
    vendorTransfer(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const TS bufferSend[], uint16_t &wLengthReceive, TR bufferReceive[])
    {
        const auto countSend = wLengthSend / sizeof(TS);
        auto payload         = make_payloadBuffer(bufferSend, countSend);
        payload.update();
        vendorTransfer(bRequest, wValue, wIndex, payload.size(), payload.data(), wLengthReceive, reinterpret_cast<uint8_t *>(bufferReceive));

        // the following should be optimized away by the compiler if target architecture is little endian
        const auto countReceive = wLengthReceive / sizeof(TR);
        littleToHost(bufferReceive, bufferReceive + countReceive);
    }

    template <typename TS, typename TR>
    void vendorTransferChecked(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLengthSend, const TS bufferSend[], uint16_t wLengthReceive, TR bufferReceive[])
    {
        const uint16_t expectedLength = wLengthReceive;
        vendorTransfer(bRequest, wValue, wIndex, wLengthSend, bufferSend, wLengthReceive, bufferReceive);
        if (wLengthReceive != expectedLength)
        {
            throw EProtocol("Unexpected transfer request response length", (expectedLength << 16) | wLengthReceive);
        }
    }
};
