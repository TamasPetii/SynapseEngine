#include "RenderBuffer.h"

namespace Syn
{
    void RenderBuffer::Initialize(const RenderBufferConfig& config)
    {
        _config = config;

        _mapped.clear();
        _gpu.clear();

        _mappedVersions.assign(_config.frames, 0);
        _gpuVersions.assign(_config.frames, 0);

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

    void RenderBuffer::Write(uint32_t frameIndex, const void* data, size_t size, size_t offset)
    {
        if (auto* buf = GetMapped(frameIndex)) {
            buf->Write(data, size, offset);
            _mappedVersions[frameIndex]++;
        }
    }

    void RenderBuffer::MarkDirty(uint32_t frameIndex)
    {
        _mappedVersions[frameIndex]++;
    }

    std::vector<std::shared_ptr<Vk::Buffer>> RenderBuffer::UpdateCapacityAll(uint64_t requiredElements)
    {
        std::vector<std::shared_ptr<Vk::Buffer>> allStale;

        bool resized = false;
        for (uint32_t i = 0; i < _config.frames; ++i) {
            StaleFrameBuffers stale = UpdateCapacity(i, requiredElements);

            if (stale.mapped) allStale.push_back(stale.mapped);
            if (stale.gpu)    allStale.push_back(stale.gpu);
        }

        return allStale;
    }

    StaleFrameBuffers RenderBuffer::UpdateCapacity(uint32_t frameIndex, uint64_t requiredElements)
    {
        StaleFrameBuffers stale{};
        bool resized = false;

        if (frameIndex < _config.frames) {
            if (_mapped[frameIndex]) {
                auto [didResize, oldBuf] = _mapped[frameIndex]->UpdateCapacity(requiredElements);
                if (didResize) {
                    stale.mapped = oldBuf;
                    resized = true;
                }
            }

            if (_gpu[frameIndex]) {
                auto [didResize, oldBuf] = _gpu[frameIndex]->UpdateCapacity(requiredElements);
                if (didResize) {
                    stale.gpu = oldBuf;
                    resized = true;
                }
            }

            if (resized) {
                _mappedVersions[frameIndex]++;
            }
        }
        return stale;
    }

    VkBuffer RenderBuffer::GetHandle(uint32_t frameIndex) const
    {
        if (_config.strategy == BufferStrategy::GpuOnly || _config.strategy == BufferStrategy::Hybrid)
            return _gpu[frameIndex]->GetBuffer()->Handle();
        else
            return _mapped[frameIndex]->GetBuffer()->Handle();
    }

    VkDeviceAddress RenderBuffer::GetAddress(uint32_t frameIndex) const
    {
        if (_config.strategy == BufferStrategy::GpuOnly || _config.strategy == BufferStrategy::Hybrid)
            return _gpu[frameIndex]->GetBuffer()->GetDeviceAddress();
        else
            return _mapped[frameIndex]->GetBuffer()->GetDeviceAddress();

    }

    Vk::Buffer* RenderBuffer::GetMapped(uint32_t frameIndex) const
    {
        return _mapped[frameIndex] ? _mapped[frameIndex]->GetBuffer() : nullptr;
    }

    Vk::Buffer* RenderBuffer::GetGpu(uint32_t frameIndex) const
    {
        return _gpu[frameIndex] ? _gpu[frameIndex]->GetBuffer() : nullptr;
    }

    void RenderBuffer::RecordSync(VkCommandBuffer cmd, uint32_t frameIndex)
    {
        if (_config.strategy == BufferStrategy::MappedOnly || _config.strategy == BufferStrategy::GpuOnly) return;
        if (!_mapped[frameIndex] || !_gpu[frameIndex]) return;
        if (_mappedVersions[frameIndex] == _gpuVersions[frameIndex]) return;

        auto* srcBuf = _mapped[frameIndex]->GetBuffer();
        auto* dstBuf = _gpu[frameIndex]->GetBuffer();

        if (!srcBuf || !dstBuf) return;

        size_t mappedSize = srcBuf->GetSize();
        size_t gpuSize = dstBuf->GetSize();
        size_t safeSize = std::min(mappedSize, gpuSize);

        if (safeSize == 0) return;

        Vk::BufferCopyInfo copyInfo{};
        copyInfo.srcBuffer = srcBuf->Handle();
        copyInfo.dstBuffer = dstBuf->Handle();
        copyInfo.size = safeSize;
        copyInfo.srcOffset = 0;
        copyInfo.dstOffset = 0;
        Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

        _gpuVersions[frameIndex] = _mappedVersions[frameIndex];
    }

    void RenderBuffer::RecordSync(VkCommandBuffer cmd, uint32_t frameIndex, size_t copySizeElements)
    {
        if (_config.strategy == BufferStrategy::MappedOnly || _config.strategy == BufferStrategy::GpuOnly || copySizeElements == 0) return;
        if (!_mapped[frameIndex] || !_gpu[frameIndex]) return;
        if (_mappedVersions[frameIndex] == _gpuVersions[frameIndex]) return;

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

        _gpuVersions[frameIndex] = _mappedVersions[frameIndex];
    }

    uint64_t RenderBuffer::GetElementCount(uint32_t frameIndex) const
    {
        if (frameIndex >= _config.frames) return 0;

        if (_config.strategy == BufferStrategy::MappedOnly) {
            return _mapped[frameIndex] ? _mapped[frameIndex]->GetCapacity() : 0;
        }
        else {
            return _gpu[frameIndex] ? _gpu[frameIndex]->GetCapacity() : 0;
        }
    }

    uint32_t RenderBuffer::GetElementSize() const
    {
        return _config.elementSize;
    }

    uint64_t RenderBuffer::GetSizeInBytes(uint32_t frameIndex) const
    {
        return GetElementCount(frameIndex) * _config.elementSize;
    }
}