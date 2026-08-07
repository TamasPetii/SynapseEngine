#include "AnnexBVideoProcessor.h"

namespace Syn
{
    void AnnexBVideoProcessor::Process(CookedVideoPacket& cookedPacket)
    {
        if (cookedPacket.data.size() < 4) return;

        if (cookedPacket.data[0] != 0x00 || cookedPacket.data[1] != 0x00 ||
            cookedPacket.data[2] != 0x00 || cookedPacket.data[3] != 0x01)
        {
            cookedPacket.data[0] = 0x00;
            cookedPacket.data[1] = 0x00;
            cookedPacket.data[2] = 0x00;
            cookedPacket.data[3] = 0x01;
        }
    }
}