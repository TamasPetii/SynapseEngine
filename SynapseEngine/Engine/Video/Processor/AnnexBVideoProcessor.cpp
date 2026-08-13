// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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