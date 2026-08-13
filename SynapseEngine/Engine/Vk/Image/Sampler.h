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
#include "../VkCommon.h"

namespace Syn::Vk {

    struct SYN_API SamplerConfig {
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkFilter minFilter = VK_FILTER_LINEAR;

        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        bool anisotropyEnable = false;
        float maxAnisotropy = 16.0f;

        VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        bool unnormalizedCoordinates = false;

        bool compareEnable = false;
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;

        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = VK_LOD_CLAMP_NONE;

        std::optional<VkSamplerReductionMode> reductionMode = std::nullopt;
    };

    class SYN_API Sampler {
    public:
        explicit Sampler(const SamplerConfig& config);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&&) = delete;
        Sampler& operator=(Sampler&&) = delete;

        VkSampler Handle() const { return _handle; }
        const SamplerConfig& GetConfig() const { return _config; }
    private:
        SamplerConfig _config;
        VkSampler _handle = VK_NULL_HANDLE;
    };
}