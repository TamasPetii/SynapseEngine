#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Utils/WindowedBuffer.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace Syn
{
    enum class SYN_API BufferStrategy {
        MappedOnly,
        GpuOnly,
        Hybrid
    };

    struct SYN_API RenderBufferConfig {
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
        bool UpdateCapacity(uint32_t frameIndex, uint64_t requiredElements);
        bool UpdateCapacityAll(uint64_t requiredElements);

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