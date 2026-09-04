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

#include "NetworkVideoSource.h"

namespace Syn
{
    NetworkVideoSource::NetworkVideoSource(const std::string& url, IVideoLoader* loader)
        : _url(url), _loader(loader)
    {
        if (_loader) {
            _state = _loader->OpenNetwork(_url);
        }
    }

    VideoInfo NetworkVideoSource::GetInfo() const
    {
        if (_state) {
            return _state->GetInfo();
        }
        return VideoInfo{};
    }

    std::optional<RawVideoPacket> NetworkVideoSource::ReadNextPacket()
    {
        if (_state) {
            return _state->ReadPacket();
        }
        return std::nullopt;
    }

    void NetworkVideoSource::Reset()
    {
        if (_state) {
            _state->Reset();
        }
    }
}