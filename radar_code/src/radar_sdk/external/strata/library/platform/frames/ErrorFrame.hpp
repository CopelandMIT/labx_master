/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once

#include "FrameBase.hpp"


class ErrorFrame :
    public FrameBase
{
private:
    ErrorFrame(uint32_t code);

public:
    static IFrame *create(uint32_t code, uint8_t virtualChannel, uint64_t timestamp = 0);

    //IFrame
    uint8_t *getData() const override;
    uint32_t getDataSize() const override;
    void setDataOffset(uint32_t offset) override;
    void setDataSize(uint32_t dataSize) override;
    void setDataOffsetAndSize(uint32_t offset, uint32_t dataSize) override;
    uint32_t getDataOffset() const override;
    uint8_t *getBuffer() const override;
    uint32_t getBufferSize() const override;
    uint32_t getStatusCode() const override;

protected:
    void queue() override;

private:
    const uint32_t m_code;
};
