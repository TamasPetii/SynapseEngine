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
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Image/Data/Texture.h"
#include "Engine/Vk/Rendering/GpuUploader.h"

namespace Syn
{
    struct SYN_API ImageUploadResult {
        std::shared_ptr<Vk::Image> texture;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        bool requiresGraphicsQueue = false;
    };

    class SYN_API IGpuImageUploader
    {
    public:
        virtual ~IGpuImageUploader() = default;
        virtual ImageUploadResult Upload(const GpuImage& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) = 0;
    };
}