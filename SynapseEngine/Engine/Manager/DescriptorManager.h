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
#include "Engine/Vk/Descriptor/DescriptorBuffer.h"
#include "Engine/Utils/StaleBuffer.h"
#include <memory>
#include <mutex>
#include <vector>

namespace Syn 
{
    class SYN_API DescriptorManager {
    public:
        static constexpr uint32_t BINDING_SAMPLERS = 0;
        static constexpr uint32_t BINDING_TEXTURES = 1;
        static constexpr uint32_t BINDING_VIDEO_TEXTURES = 2;
        static constexpr uint32_t BINDING_CUBE_TEXTURES = 3;

        static constexpr uint32_t MAX_SAMPLERS = 32;
        static constexpr uint32_t MAX_IMAGES = 2048;
        static constexpr uint32_t MAX_VIDEOS = 64;
        static constexpr uint32_t MAX_ENVIRONMENTS = 1024;

        DescriptorManager(uint32_t framesInFlight);
        ~DescriptorManager();

        void Update();
        void RecordSync(VkCommandBuffer cmd);

        void WriteSampler(uint32_t index, VkSampler sampler);
        void WriteTexture(uint32_t index, VkImageView view);
        void FillTextures(VkImageView defaultView);
        void WriteVideoTexture(uint32_t index, VkImageView view);
        void WriteCubeTexture(uint32_t index, VkImageView view);

        Vk::DescriptorBuffer* GetBindlessBuffer() const { return _bindlessBuffer.get(); }
        VkDescriptorSetLayout GetBindlessLayout() const { return _bindlessLayout; }
    private:
        void InitializeBindlessSetup();
    private:
        uint32_t _framesInFlight;
        VkDescriptorSetLayout _bindlessLayout = VK_NULL_HANDLE;
        std::unique_ptr<Vk::DescriptorBuffer> _bindlessBuffer;

        std::mutex _staleMutex;
        std::vector<StaleBuffer> _staleGpuBuffers;
        std::vector<StaleBuffer> _staleMappedBuffers;
    };
}