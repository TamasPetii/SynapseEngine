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

#include "FileVideoSource.h"

namespace Syn
{
    FileVideoSource::FileVideoSource(const std::filesystem::path& path, IVideoLoader* loader)
        : _path(path), _loader(loader)
    {
        if (_loader) {
            _state = _loader->OpenFile(_path);
        }
    }

    VideoInfo FileVideoSource::GetInfo() const
    {
        if (_state) {
            return _state->GetInfo();
        }
        return VideoInfo{};
    }

    std::optional<RawVideoPacket> FileVideoSource::ReadNextPacket()
    {
        if (_state) {
            return _state->ReadPacket();
        }
        return std::nullopt;
    }

    void FileVideoSource::Reset()
    {
        if (_state) {
            _state->Reset();
        }
    }
}