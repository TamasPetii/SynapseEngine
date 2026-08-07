#include "AnnexBVideoProcessor.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    void AnnexBVideoProcessor::Process(CookedVideoPacket& cookedPacket)
    {
        if (cookedPacket.data.size() < 4) return;

        std::vector<uint8_t> annexbData;
        annexbData.reserve(cookedPacket.data.size() + 16);

        size_t offset = 0;
        size_t size = cookedPacket.data.size();

        while (offset + 4 <= size) {
            uint32_t naluLen = (static_cast<uint32_t>(cookedPacket.data[offset]) << 24) |
                (static_cast<uint32_t>(cookedPacket.data[offset + 1]) << 16) |
                (static_cast<uint32_t>(cookedPacket.data[offset + 2]) << 8) |
                (static_cast<uint32_t>(cookedPacket.data[offset + 3]));

            offset += 4;

            if (naluLen == 0 || offset + naluLen > size) {
                break;
            }

            annexbData.push_back(0x00);
            annexbData.push_back(0x00);
            annexbData.push_back(0x00);
            annexbData.push_back(0x01);

            annexbData.insert(annexbData.end(),
                cookedPacket.data.begin() + offset,
                cookedPacket.data.begin() + offset + naluLen);

            offset += naluLen;
        }

        if (!annexbData.empty()) {
            cookedPacket.data = std::move(annexbData);
        }
    }
}