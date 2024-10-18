#include "FrameHelper.hpp"
#include <common/Packed12.hpp>

void unpackPacked12(IFrame *frame)
{
    auto buf         = reinterpret_cast<uint16_t *>(frame->getData());
    auto length      = frame->getDataSize();
    const auto count = length * 2 / 3;

    length = count * sizeof(*buf);

    frame->setDataSize(length);
    unpackPacked12(buf, buf + count);
}
