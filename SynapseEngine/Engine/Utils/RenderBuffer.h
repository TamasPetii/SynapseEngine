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
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Utils/WindowedBuffer.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace Syn
{
    struct SYN_API StaleFrameBuffers {
        std::shared_ptr<Vk::Buffer> mapped;
        std::shared_ptr<Vk::Buffer> gpu;

        bool HasAny() const { return mapped != nullptr || gpu != nullptr; }
    };

    enum class SYN_API BufferStrategy {
        MappedOnly,
        GpuOnly,
        Hybrid
    };

    struct SYN_API RenderBufferConfig {
        std::string debugName = "";
        BufferStrategy strategy = BufferStrategy::Hybrid;
        uint32_t frames = 0;
        uint32_t elementSize = 0;
        VkBufferUsageFlags usage = 0;
        uint32_t upWindow = 256;
        uint32_t downWindow = 512;
    };

    class SYN_API RenderBuffer
    {
    public:
        RenderBuffer() = default;

        RenderBuffer(const RenderBuffer& other) = delete;
        RenderBuffer& operator=(const RenderBuffer& other) = delete;

        RenderBuffer(RenderBuffer&& other) noexcept = default;
        RenderBuffer& operator=(RenderBuffer&& other) noexcept = default;

        void Initialize(const RenderBufferConfig& config);
        StaleFrameBuffers UpdateCapacity(uint32_t frameIndex, uint64_t requiredElements);
        std::vector<std::shared_ptr<Vk::Buffer>> UpdateCapacityAll(uint64_t requiredElements);

        void RecordSync(VkCommandBuffer cmd, uint32_t frameIndex);
        void RecordSync(VkCommandBuffer cmd, uint32_t frameIndex, size_t copySizeElements);

        Vk::Buffer* GetMapped(uint32_t frameIndex) const;
        Vk::Buffer* GetGpu(uint32_t frameIndex) const;

        uint64_t GetElementCount(uint32_t frameIndex) const;
        uint32_t GetElementSize() const;
        uint64_t GetSizeInBytes(uint32_t frameIndex) const;

        VkBuffer GetHandle(uint32_t frameIndex) const;
        VkDeviceAddress GetAddress(uint32_t frameIndex) const;

        void Write(uint32_t frameIndex, const void* data, size_t size, size_t offset = 0);
        void MarkDirty(uint32_t frameIndex);
    private:
        RenderBufferConfig _config;
        std::vector<std::unique_ptr<WindowedBuffer>> _mapped;
        std::vector<std::unique_ptr<WindowedBuffer>> _gpu;

        std::vector<uint32_t> _mappedVersions;
        std::vector<uint32_t> _gpuVersions;
    };
}