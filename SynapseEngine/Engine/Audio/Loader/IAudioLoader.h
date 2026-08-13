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

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Raw/RawAudio.h"
#include <optional>
#include <filesystem>
#include <string>
#include <vector>

namespace Syn
{
    class SYN_API IAudioLoader {
    public:
        virtual ~IAudioLoader() = default;
        virtual std::optional<RawAudio> LoadFile(const std::filesystem::path& path) = 0;
        virtual std::optional<RawAudio> LoadMemory(const std::vector<uint8_t>& data) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}