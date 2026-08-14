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
#include "Engine/Animation/Data/Gpu/GpuAnimationBuffers.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include <memory>

namespace Syn
{
    struct SYN_API AnimationUploadResult {
        AnimationUploadResult() = default;
        GpuAnimationBuffers hardwareBuffers;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
    };

    class SYN_API IGpuAnimationUploader
    {
    public:
        virtual ~IGpuAnimationUploader() = default;
        virtual AnimationUploadResult Upload(const GpuBatchedAnimation& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) = 0;
    };
}