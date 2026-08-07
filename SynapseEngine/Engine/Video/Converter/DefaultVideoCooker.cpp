#include "DefaultVideoCooker.h"

namespace Syn
{
    CookedVideoPacket DefaultVideoCooker::Cook(const RawVideoPacket& rawPacket)
    {
        CookedVideoPacket cookedPacket{};

        cookedPacket.data = rawPacket.data;
        cookedPacket.pts = rawPacket.pts;
        cookedPacket.dts = rawPacket.dts;
        cookedPacket.isKeyFrame = rawPacket.isKeyFrame;
        cookedPacket.format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

        return cookedPacket;
    }
}