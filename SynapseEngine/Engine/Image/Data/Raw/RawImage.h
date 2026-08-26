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
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <functional>
#include "Engine/Image/Data/Common/MipLevelInfo.h"
#include "Engine/Vk/Image/Image.h"

namespace Syn
{
    struct SYN_API RawImage {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;

        VkFormat format = VK_FORMAT_UNDEFINED;

        bool isCompressed = false;
        bool isGpuGenerated = false;
        bool autoCache = false;

        std::vector<uint8_t> pixels;
        std::vector<MipLevelInfo> mipData;
        std::function<void(VkCommandBuffer, class Vk::Image&)> gpuGeneratorCallback;
    };
}