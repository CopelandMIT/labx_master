/**
 * @copyright 2021 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <Library.hpp>
#include <common/Packed12.hpp>
#include <common/cpp11/iterator.hpp>
#include <components/interfaces/IRadarAvian.hpp>
#include <platform/BoardManager.hpp>
#include <platform/interfaces/IFrameListener.hpp>
#include <universal/types/DataSettingsBgtRadar.h>

#include <iomanip>
#include <iostream>
#include <thread>


#include "config.hpp"

const uint8_t dataFormat = DataFormat_Auto;  // DataFormat_Packed12;
const bool useCallback   = false;


using namespace std;


class FrameListener :
    public IFrameListener<>
{
public:
    void onNewFrame(IFrame *frame) override
    {
        auto code = frame->getStatusCode();
        if (code)
        {
            cout << "Frame error: 0x" << std::hex << code << std::endl;
            return;
        }

        auto buf    = reinterpret_cast<uint16_t *>(frame->getData());
        auto length = frame->getDataSize();

        uint32_t sampleCount;
        if (dataFormat == DataFormat_Auto)
        {
            sampleCount = length * 2 / 3;
            unpackPacked12(buf, buf + sampleCount);
        }
        else
        {
            sampleCount = length / 2;
        }

        cout << "Frame size: " << std::dec << length << " - timestamp: " << std::fixed << std::setprecision(1) << (frame->getTimestamp() / 1000000.0) << " - id: " << std::hex << static_cast<int>(((buf[1] & 0x000F) << 8) + buf[0]) << " - address: " << reinterpret_cast<uintptr_t>(buf) << endl;
        frame->release();
        m_count++;
    }

    volatile int m_count = 0;
};


int main(int argc, char *argv[])
{
    FrameListener listener;

    cout << "Starting application example..." << endl;

    cout << "Strata version: " << Strata::getVersionString();

    cout << "1) trying to connect to a board ..." << endl;
    BoardManager boardManager;
    boardManager.useSerial().useUdp().useLibusb();
    boardManager.enumerate();

    //auto board       = boardManager.createBoardInstance();  // connect to any found board

    //auto board       = boardManager.createBoardInstance(1);  // connect to the second found board (with index == 1)

    //    const uint8_t uuid[UUID_LENGTH] = {};
    //    auto board         = boardManager.createSpecificBoardInstance(uuid);  // connect to the first specified board forund

    // Hatvan PID/VID:
    const uint16_t vid = 0x058b;
    const uint16_t pid = 0x0251;
    auto board         = boardManager.createBoardInstance(vid, pid);  // connect to the first specified board forund

    cout << " ... Board Firmware Version: " << board->getVersionString() << endl;


    cout << "2) get the interface to a remote sensor running in the FW of the board..." << endl;

    const uint8_t componentId = 0;

    auto avian = board->getComponent<IRadarAvian>(componentId);
    auto cmd   = avian->getIProtocolAvian();

    auto data            = board->getIData();
    const auto dataIndex = avian->getDataIndex();


    cout << "3) configure data readout and sensor..." << endl;

    // configure data readout
    IDataProperties_t properties;
    properties.format            = dataFormat;
    const uint16_t readouts[][2] = {
        meta_data.burstAddress & 0xFF,
        meta_data.burstSize & 0xFFFF,
    };
    DataSettingsBgtRadar_t settings(&readouts);
    data->configure(dataIndex, &properties, &settings);

    // configure host frame buffers
    auto bridgeData                = board->getIBridge()->getIBridgeData();
    const uint32_t frameBuffersize = meta_data.burstSize * sizeof(uint16_t);  // size suits both packed and unpacked raw data
    bridgeData->setFrameBufferSize(frameBuffersize);
    bridgeData->setFrameQueueSize(20);

    if (useCallback)
    {
        bridgeData->registerListener(&listener);
    }

    // start data channel communication over protocol
    bridgeData->startStreaming();

    // start data readout statemachine
    data->start(dataIndex);

    // configure register map which also starts the sensor readout (data interface readout has to be started already!)
    cmd->execute(default_doppler);

    while (listener.m_count < 5)
    {
        if (!useCallback)
        {
            const uint16_t timeoutMs = 1000;

            auto frame = board->getFrame(timeoutMs);
            if (frame)
            {
                listener.onNewFrame(frame);
            }
        }
    }

    // stop data channel communication over protocol
    bridgeData->stopStreaming();

    // stop data acquisition (this will lead to a FIFO overflow in the sensor, which will stop its state machine)
    data->stop(dataIndex);

    cout << "4) Restart data readout..." << endl;

    // restart sensor again

    // start data channel communication over protocol
    bridgeData->startStreaming();

    // start data readout statemachine
    data->start(dataIndex);

    // configure register map which also starts the sensor readout (data interface readout has to be started already!)
    cmd->execute(default_doppler);

    while (listener.m_count < 10)
    {
        if (!useCallback)
        {
            const uint16_t timeoutMs = 1000;

            auto frame = board->getFrame(timeoutMs);
            if (frame)
            {
                listener.onNewFrame(frame);
            }
        }
    }

    // stop data channel communication over protocol
    bridgeData->stopStreaming();

    // stop data acquisition (this will lead to a FIFO overflow in the sensor, which will stop its state machine)
    data->stop(dataIndex);

    return 0;
}
