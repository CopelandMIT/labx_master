/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "BridgeFpgaIrpli.hpp"

#include <platform/bridge/BridgeProtocol.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EProtocol.hpp>

#include <common/BinUtils.hpp>
#include <common/Logger.hpp>
#include <common/crc/Crc32.hpp>
#include <fstream>


namespace
{
    const char fpgaFilename[] = "C:\\ARS\\fpga_project\\final\\LVDS.bin";
    const uint32_t bufSize    = 4096;


    void ReverseData32Bit(uint8_t data[], uint32_t count)
    {
        //extend the length so the last group is also 32 bits (= 4 bytes)
        unsigned int configLength = ((count + 3) / 4) * 4;
        if (configLength > bufSize)
        {
            throw EConnection("Error preparing FPGA bitstream data");
        }

        //iterate over the groups of 4 bytes
        for (unsigned int i = 0; i < count; i += 4)
        {
            unsigned int val = 0;
            //take 4 bytes
            for (unsigned int j = 0; j < 4; j++)
            {
                unsigned int index = i + j;
                if (index < count)
                {
                    val |= static_cast<unsigned int>(data[index] << (8 * j));
                }
            }

            //reverse the bits of the value
            //swap single bits
            val = ((val & 0xAAAAAAAA) >> 1) | ((val & 0x55555555) << 1);
            //swap bit pairs
            val = ((val & 0xCCCCCCCC) >> 2) | ((val & 0x33333333) << 2);
            //swap 4-bit groups
            val = ((val & 0xF0F0F0F0) >> 4) | ((val & 0x0F0F0F0F) << 4);
            //swap bytes
            val = ((val & 0xFF00FF00) >> 8) | ((val & 0x00FF00FF) << 8);
            //swap byte pairs
            val = (val >> 16) | (val << 16);

            //save bytes into config data array
            for (unsigned int j = 0; j < 4; j++)
            {
                data[i + j] = static_cast<uint8_t>((val >> (8 * j)) & 0xFF);
            }
        }
    }
}


BridgeFpgaIrpli::BridgeFpgaIrpli(std::shared_ptr<IBridge> &&bridge) :
    m_bridge {std::move(bridge)},
    m_swapping {0}
{
    m_bridgeData = m_bridge->getIBridgeData();
    m_commands   = m_bridge->getIBridgeControl()->getIVendorCommands();
    if (m_commands == nullptr)
    {
        throw EProtocol("There's no vendor implementation in this Bridge");
    }

    // reset FPGA if it is already programmed
    //    auto gpio = m_bridge->getIBridgeControl()->getIGpio();
    //    gpio->setPin(54, false);
    //    gpio->setPin(54, true);

    if (!getFpgaDonePin())
    {
        std::ifstream bitstreamFile(fpgaFilename, std::ios::in | std::ios::binary | std::ios::ate);
        if (!bitstreamFile.is_open())
        {
            throw EConnection("Can not open FPGA bitstream file");
        }

        try
        {
            uint32_t count = static_cast<uint32_t>(bitstreamFile.tellg());
            bitstreamFile.seekg(0, std::ios::beg);
            uint32_t remaining = count;
            uint8_t buf[bufSize];
            while (remaining)
            {
                const uint16_t chunkSize = (remaining > bufSize) ? bufSize : static_cast<uint16_t>(remaining);
                bitstreamFile.read(reinterpret_cast<char *>(buf), chunkSize);
                if (bitstreamFile.gcount() != chunkSize)
                {
                    throw EConnection("Error reading FPGA bitstream file");
                }
                ReverseData32Bit(buf, chunkSize);
                sendFpgaBuffer(count, buf, chunkSize);
                remaining -= chunkSize;
                count = 0;  // set count parameter to zero for all follow-up calls
            }
        }
        catch (...)
        {
            bitstreamFile.close();
            throw EConnection("FPGA configuration error");
        }
        LOG(DEBUG) << "FPGA configuration successful";

        bitstreamFile.close();
    }

    m_bridgeData->registerListener(this);
}

void BridgeFpgaIrpli::setChannelSwapping(uint8_t swapping)
{
    m_swapping = swapping;
}

void BridgeFpgaIrpli::setDataProperties(IDataProperties_t *props)
{
    m_properties = *props;
}

bool BridgeFpgaIrpli::isConnected()
{
    return m_bridge->isConnected();
}

void BridgeFpgaIrpli::openConnection()
{
    m_bridge->openConnection();
}

void BridgeFpgaIrpli::closeConnection()
{
    m_bridge->closeConnection();
}

IBridgeControl *BridgeFpgaIrpli::getIBridgeControl()
{
    return m_bridge->getIBridgeControl();
}

IBridgeData *BridgeFpgaIrpli::getIBridgeData()
{
    return this;
}

void BridgeFpgaIrpli::startStreaming()
{
    m_bridgeData->startStreaming();
}

void BridgeFpgaIrpli::stopStreaming()
{
    m_bridgeData->stopStreaming();
}

void BridgeFpgaIrpli::setFrameBufferSize(uint32_t size)
{
    m_bridgeData->setFrameBufferSize(size);
}

void BridgeFpgaIrpli::setFrameQueueSize(uint16_t count)
{
    m_bridgeData->setFrameQueueSize(count);
}

void BridgeFpgaIrpli::clearFrameQueue()
{
    m_bridgeData->clearFrameQueue();
}

void BridgeFpgaIrpli::onNewFrame(IFrame *frame)
{
    //    const bool crcWorkaround = false;
    const bool crcWorkaround = true;  // fix for ES2 bug
    const uint32_t crcCount  = 2;
    // always assume CRC is on

    auto buf = frame->getData();
    //const auto length = frame->getDataSize();

    // assume uint16_t
    const auto count = frame->getDataSize() / 2;
    auto samples     = reinterpret_cast<uint16_t *>(buf);

    const auto channels = m_properties.rxChannels;
    //    const auto ramps = m_properties.ramps;
    const uint_fast16_t sampleCount = m_properties.samples;
    // const uint_fast16_t sampleCount = // derive sample count from ramp size
    const uint_fast16_t ramps             = 1;  // one ramp per frame
    const uint_fast16_t samplesPerChannel = (sampleCount + crcCount) * ramps;

    const uint32_t expectedCount = samplesPerChannel * channels;

    if (count != expectedCount)
    {
        LOG(ERROR) << "BridgeFpgaIrpli - count mismatch, expected: " << std::dec << expectedCount << " received: " << count;

        //throw EProtocol("Unexpected amount of samples received", static_cast<int>(count));
    }

    // change sample ordering
    reshape(samples, samplesPerChannel, channels);

    // check CRC
    //const auto last = samples + count;
    auto s = samples;

    for (unsigned int i = 0; i < channels; i++)
    {
        auto crc = Crc32Mpeg2(s, sampleCount, m_properties.bitWidth);
        s += sampleCount;
        if (crcWorkaround)
        {
            crc = Crc32Mpeg2(s + 1, 1, 16, crc);
            crc = Crc32Mpeg2(s, 1, 16, crc);
        }
        else
        {
            crc = Crc32Mpeg2(s, 2, 16, crc);
        }
        if (crc)
        {
            LOG(INFO) << "BridgeFpgaIrpli - CRC Error detected on channel: " << i << " - 0x" << std::hex << crc;
        }
        else
        {
            LOG(INFO) << "BridgeFpgaIrpli - no error on channel: " << i;
        }

        // copy next ramp to forwarding frame buffer

        s += 2;
    }

    // forward frame buffer...
    // for now, just do one ramp per frame
    // todo: allocate buffer pool to hold complete ramp sequence and copy over data

    m_caller.callListener(frame);
}

void BridgeFpgaIrpli::registerListener(IFrameListener<> *listener)
{
    m_caller.registerListener(listener);
}

IFrame *BridgeFpgaIrpli::getFrame(uint16_t /*timeoutMs*/)
{
    return nullptr;  //getting frames is not supported by this bridge, todo: when allocating own pool, do this
}

bool BridgeFpgaIrpli::getFpgaDonePin()
{
    uint8_t buf[1];
    m_commands->vendorRead(0x0F,
                           0,
                           0,
                           1,
                           buf);
    return buf[0];
}

void BridgeFpgaIrpli::sendFpgaBuffer(uint32_t count, const uint8_t buf[], uint16_t length)
{
    m_commands->vendorWrite(0x0F,
                            static_cast<uint16_t>(count),
                            count >> 16,
                            length,
                            buf);
}
