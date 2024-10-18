/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include <iostream>

#include <platform/BoardManager.hpp>
#include <platform/interfaces/IFrameListener.hpp>

#include <iomanip>
#include <iostream>
#include <thread>


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

        const auto buf    = frame->getData();
        const auto length = frame->getDataSize();
        cout << "Frame size: " << std::dec << length << " - timestamp: " << std::fixed << std::setprecision(1) << (frame->getTimestamp() / 1000000.0) << " - id: " << std::hex << static_cast<int>(((buf[1] & 0x000F) << 8) + buf[0]) << " - address: " << reinterpret_cast<uintptr_t>(buf) << endl;
        frame->release();
        m_count++;
    }

    volatile int m_count = 0;
};

static FrameListener listener;
static const bool useCallback = false;

int main(int argc, char *argv[])
{
    BoardManager boardMgr(true, true);
    boardMgr.enumerate();
    auto board = boardMgr.createBoardInstance();

    // get component interface
    //auto radar       = board->getComponent<IRadar>(0);
    //auto registers = radar->getIRegisters();

    auto bridgeData                = board->getIBridge()->getIBridgeData();
    const uint32_t frameBuffersize = 1024 * sizeof(uint16_t);
    bridgeData->setFrameBufferSize(frameBuffersize);
    bridgeData->setFrameQueueSize(20);

    auto data               = board->getIData();
    const uint8_t dataIndex = 0;

    const bool useCallback = false;
    if (useCallback)
    {
        bridgeData->registerListener(&listener);
    }

    // start data channel communication over protocol
    bridgeData->startStreaming();

    // start data readout statemachine
    data->start(dataIndex);

    // start radar sensor

    while (listener.m_count < 10)
    {
        if (!useCallback)
        {
            auto frame = board->getFrame();
            if (frame)
            {
                listener.onNewFrame(frame);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // stop data channel communication over protocol
    bridgeData->stopStreaming();

    // stop data acquisition
    data->stop(dataIndex);

    return 0;
}
