#include "RenderBuffer.h"

namespace Syn
{
    void RenderBuffer::Initialize(const RenderBufferConfig& config)
    {
        _config = config;

        _mapped.clear();
        _gpu.clear();

        Vk::BufferConfig mappedConfig{};
        mappedConfig.usage = _config.usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        mappedConfig.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        mappedConfig.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        mappedConfig.useDeviceAddress = (_config.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0;

        Vk::BufferConfig gpuConfig{};
        gpuConfig.usage = _config.usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        gpuConfig.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        gpuConfig.allocationFlags = 0;
        gpuConfig.useDeviceAddress = (_config.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0;

        for (uint32_t i = 0; i < _config.frames; ++i) {
            if (_config.strategy != BufferStrategy::GpuOnly) {
                _mapped.push_back(std::make_unique<WindowedBuffer>(mappedConfig, _config.elementSize, _config.upWindow, _config.downWindow));
            }
            else {
                _mapped.push_back(nullptr);
            }

            if (_config.strategy != BufferStrategy::MappedOnly) {
                _gpu.push_back(std::make_unique<WindowedBuffer>(gpuConfig, _config.elementSize, _config.upWindow, _config.downWindow));
            }
            else {
                _gpu.push_back(nullptr);
            }
        }
    }

    bool RenderBuffer::UpdateCapacityAll(uint32_t requiredElements)
    {
        bool resized = false;
        for (uint32_t i = 0; i < _config.frames; ++i) {
            if (UpdateCapacity(i, requiredElements)) {
                resized = true;
            }
        }
        return resized;
    }

    bool RenderBuffer::UpdateCapacity(uint32_t frameIndex, uint32_t requiredElements)
    {
        bool resized = false;
        if (frameIndex < _config.frames) {
            if (_mapped[frameIndex] && _mapped[frameIndex]->UpdateCapacity(requiredElements)) resized = true;
            if (_gpu[frameIndex] && _gpu[frameIndex]->UpdateCapacity(requiredElements)) resized = true;
        }
        return resized;
    }

    VkBuffer RenderBuffer::GetHandle(uint32_t frameIndex, bool useGpuDriven) const
    {
        if (_config.strategy == BufferStrategy::GpuOnly || _config.strategy == BufferStrategy::Hybrid_Static) {
            return _gpu[frameIndex]->GetBuffer()->Handle();
        }
        if (_config.strategy == BufferStrategy::MappedOnly) {
            return _mapped[frameIndex]->GetBuffer()->Handle();
        }
        return useGpuDriven ? _gpu[frameIndex]->GetBuffer()->Handle() : _mapped[frameIndex]->GetBuffer()->Handle();
    }

    VkDeviceAddress RenderBuffer::GetAddress(uint32_t frameIndex, bool useGpuDriven) const
    {
        if (_config.strategy == BufferStrategy::GpuOnly || _config.strategy == BufferStrategy::Hybrid_Static) {
            return _gpu[frameIndex]->GetBuffer()->GetDeviceAddress();
        }
        if (_config.strategy == BufferStrategy::MappedOnly) {
            return _mapped[frameIndex]->GetBuffer()->GetDeviceAddress();
        }
        return useGpuDriven ? _gpu[frameIndex]->GetBuffer()->GetDeviceAddress() : _mapped[frameIndex]->GetBuffer()->GetDeviceAddress();
    }

    Vk::Buffer* RenderBuffer::GetMapped(uint32_t frameIndex) const
    {
        return _mapped[frameIndex] ? _mapped[frameIndex]->GetBuffer() : nullptr;
    }

    Vk::Buffer* RenderBuffer::GetGpu(uint32_t frameIndex) const
    {
        return _gpu[frameIndex] ? _gpu[frameIndex]->GetBuffer() : nullptr;
    }

    void RenderBuffer::RecordSync(VkCommandBuffer cmd, uint32_t frameIndex, size_t copySizeElements)
    {
        if (_config.strategy == BufferStrategy::MappedOnly || _config.strategy == BufferStrategy::GpuOnly || copySizeElements == 0) return;
        if (!_mapped[frameIndex] || !_gpu[frameIndex]) return;

        auto* srcBuf = _mapped[frameIndex]->GetBuffer();
        auto* dstBuf = _gpu[frameIndex]->GetBuffer();

        if (!srcBuf || !dstBuf) return;

        size_t copySizeBytes = copySizeElements * _config.elementSize;
        size_t mappedSize = srcBuf->GetSize();
        size_t gpuSize = dstBuf->GetSize();
        size_t safeSize = std::min({ copySizeBytes, mappedSize, gpuSize });

        if (safeSize == 0) return;

        Vk::BufferCopyInfo copyInfo{};
        copyInfo.srcBuffer = srcBuf->Handle();
        copyInfo.dstBuffer = dstBuf->Handle();
        copyInfo.size = safeSize;
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);
    }
}