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
#include "IAudioLoader.h"

namespace Syn
{
    class SYN_API MiniAudioLoader : public IAudioLoader
    {
    public:
        MiniAudioLoader() = default;
        ~MiniAudioLoader() override = default;

        std::optional<RawAudio> LoadFile(const std::filesystem::path& path) override;
        std::optional<RawAudio> LoadMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}