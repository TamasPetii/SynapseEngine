#include "ComponentBufferManager.h"

namespace Syn
{
    ComponentBufferManager::ComponentBufferManager(uint32_t frameCount)
        : _frameCount(frameCount)
    {}

    void ComponentBufferManager::RegisterBuffer(const std::string& name, uint32_t elementSize, std::function<uint32_t()> sizeCallback)
    {
        Vk::BufferConfig config;
        config.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = true;

        ComponentBufferState state;
        state.sizeCallback = std::move(sizeCallback);

        for (uint32_t i = 0; i < _frameCount; ++i)
        {
            state.frames.push_back({
                .gpuBuffer = WindowedBuffer(config, elementSize, 256, 512)
                });
        }

        _buffers[name] = std::move(state);
    }

    void ComponentBufferManager::Update(uint32_t frameIndex)
    {
        for (auto& [name, state] : _buffers)
        {
            uint32_t requiredSize = state.sizeCallback();
            auto& frameData = state.frames[frameIndex];

            if (frameData.gpuBuffer.UpdateCapacity(requiredSize))
            {
                frameData.versions.resize(frameData.gpuBuffer.GetCapacity(), 0);
            }
        }
    }

    ComponentBufferView ComponentBufferManager::GetComponentBuffer(const std::string& name, uint32_t frameIndex)
    {
        auto it = _buffers.find(name);

        if (it != _buffers.end())
        {
            auto& frameData = it->second.frames[frameIndex];
            return { frameData.gpuBuffer.GetBuffer(), frameData.versions };
        }

        return { nullptr, std::span<uint32_t>() };
    }
}