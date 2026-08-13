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