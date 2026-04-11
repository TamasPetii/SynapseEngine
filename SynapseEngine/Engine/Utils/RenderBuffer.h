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
        Hybrid_Dynamic,
        Hybrid_Static
    };

    struct SYN_API RenderBufferConfig {
        BufferStrategy strategy = BufferStrategy::Hybrid_Dynamic;
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
        bool UpdateCapacity(uint32_t frameIndex, uint32_t requiredElements);
        bool UpdateCapacityAll(uint32_t requiredElements);
        void RecordSync(VkCommandBuffer cmd, uint32_t frameIndex, size_t copySizeElements);

        Vk::Buffer* GetMapped(uint32_t frameIndex) const;
        Vk::Buffer* GetGpu(uint32_t frameIndex) const;

        VkBuffer GetHandle(uint32_t frameIndex, bool useGpuDriven) const;
        VkDeviceAddress GetAddress(uint32_t frameIndex, bool useGpuDriven) const;
    private:
        RenderBufferConfig _config;
        std::vector<std::unique_ptr<WindowedBuffer>> _mapped;
        std::vector<std::unique_ptr<WindowedBuffer>> _gpu;
    };
}