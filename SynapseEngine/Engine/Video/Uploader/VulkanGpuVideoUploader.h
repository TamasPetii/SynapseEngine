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
#include "IGpuVideoUploader.h"
#include "Engine/Video/Parser/IH264ExtradataParser.h"
#include <vector>
#include <memory>

namespace Syn
{
    class SYN_API VulkanGpuVideoUploader : public IGpuVideoUploader
    {
    public:
        VulkanGpuVideoUploader(
            uint32_t width,
            uint32_t height,
            uint32_t bufferCount,
            const std::vector<uint8_t>& extradata,
            std::shared_ptr<IH264ExtradataParser> parser
        );
        ~VulkanGpuVideoUploader() override;

        VideoUploadResult Upload(const GpuVideoPacket& data, VkCommandBuffer cmd) override;
    private:
        uint32_t _width;
        uint32_t _height;
        std::vector<uint8_t> _extradata;
        std::shared_ptr<IH264ExtradataParser> _parser;

        uint8_t _spsId = 0;
        uint8_t _ppsId = 0;

        VkVideoSessionKHR _videoSession = VK_NULL_HANDLE;
        VkVideoSessionParametersKHR _sessionParams = VK_NULL_HANDLE;
        VkSamplerYcbcrConversion _ycbcrConversion = VK_NULL_HANDLE;
        std::vector<VkDeviceMemory> _sessionMemories;

        std::vector<std::shared_ptr<Vk::Image>> _textures;
        uint32_t _frameIndex = 0;
    };
}