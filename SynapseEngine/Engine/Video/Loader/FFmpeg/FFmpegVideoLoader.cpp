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

#include "FFmpegVideoLoader.h"

namespace Syn
{
    std::unique_ptr<IVideoState> FFmpegVideoLoader::OpenFile(const std::filesystem::path& path)
    {
        auto state = std::make_unique<FFmpegVideoState>(path.string());
        if (state->GetInfo().width == 0) {
            return nullptr;
        }
        return state;
    }

    std::unique_ptr<IVideoState> FFmpegVideoLoader::OpenMemory(const std::vector<uint8_t>& data)
    {
        return nullptr;
    }

    std::unique_ptr<IVideoState> FFmpegVideoLoader::OpenNetwork(const std::string& url)
    {
        auto state = std::make_unique<FFmpegVideoState>(url);
        if (state->GetInfo().width == 0) {
            return nullptr;
        }
        return state;
    }

    std::vector<std::string> FFmpegVideoLoader::GetSupportedExtensions() const
    {
        return { ".mp4", ".mkv", ".avi", ".mov", ".webm" };
    }
}