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
#include "Engine/Video/Uploader/IGpuVideoUploader.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API CpuPixelVideoUploader : public IGpuVideoUploader
    {
    public:
        CpuPixelVideoUploader(uint32_t width, uint32_t height, uint32_t bufferCount);
        ~CpuPixelVideoUploader() override = default;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) override;
    private:
        std::vector<std::shared_ptr<Vk::Image>> _textures;
        uint32_t _frameIndex = 0;

        uint32_t _width;
        uint32_t _height;
    };
}