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
#include <string>
#include <vector>
#include "IApi.h"
#include "Engine/Image/Data/Cpu/CpuTextureData.h"
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    constexpr uint32_t INVALID_TEXTURE_ID = 0xFFFFFFFF;
    constexpr uint32_t INVALID_SAMPLER_ID = 0xFFFFFFFF;

    struct TextureItemData {
        uint32_t id;
        std::string name;
        std::string path;
        TextureHandle handle;
    };

    struct SamplerItemData {
        uint32_t id;
        std::string name;
    };

    class ITextureApi : public IApi {
    public:
        virtual ~ITextureApi() = default;

        virtual std::vector<TextureItemData> GetAllTextures() = 0;
        virtual std::vector<SamplerItemData> GetAllSamplers() const = 0;

        virtual uint32_t GetSelectedTexture() const = 0;
        virtual void SetSelectedTexture(uint32_t id) = 0;
        virtual bool GetTextureData(uint32_t id, CpuTextureData& outData) const = 0;
        virtual uint64_t GetVersion() const = 0;
        virtual TextureHandle GetTextureHandle(uint32_t id) = 0;
    };
}