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

#include "ProceduralVideoSource.h"

namespace Syn
{
    ProceduralVideoSource::ProceduralVideoSource(const VideoInfo& info)
        : _info(info), _currentFrame(0)
    {}

    VideoInfo ProceduralVideoSource::GetInfo() const
    {
        return _info;
    }

    std::optional<RawVideoPacket> ProceduralVideoSource::ReadNextPacket()
    {
        if (_info.duration > 0 && _currentFrame >= _info.duration) {
            return std::nullopt;
        }

        RawVideoPacket packet;
        packet.pts = _currentFrame;
        packet.dts = _currentFrame;
        packet.isKeyFrame = (_currentFrame % 30 == 0);
        packet.data.resize(1024, 0);

        _currentFrame++;
        return packet;
    }

    void ProceduralVideoSource::Reset()
    {
        _currentFrame = 0;
    }
}