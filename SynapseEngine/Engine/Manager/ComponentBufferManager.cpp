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

#include "ComponentBufferManager.h"

namespace Syn
{
    ComponentBufferManager::ComponentBufferManager(uint32_t frameCount)
        : _frameCount(frameCount)
    {}

    void ComponentBufferManager::RegisterBuffer(const std::string& name, uint32_t elementSize, std::function<uint32_t()> sizeCallback, std::function<bool()> readyCallback, ComponentMemoryType memoryType)
    {
        Vk::BufferConfig config;
		config.debugName = name;
        config.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        config.useDeviceAddress = true;

        if (memoryType == ComponentMemoryType::Persistent) {
            config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
            config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        }
        else {
            config.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            config.allocationFlags = 0;
            config.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        ComponentBufferState state;
        state.sizeCallback = std::move(sizeCallback);
        state.readyCallback = std::move(readyCallback);
        state.memoryType = memoryType;

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

            auto [resized, oldBuffer] = frameData.gpuBuffer.UpdateCapacity(requiredSize);

            if (resized)
                frameData.versions.resize(frameData.gpuBuffer.GetCapacity(), 0);
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

    uint64_t ComponentBufferManager::GetBufferAddr(const std::string& name, uint32_t frameIndex)
    {
        auto it = _buffers.find(name);

        if (it != _buffers.end())
        {
            if (it->second.readyCallback && !it->second.readyCallback()) {
                return 0;
            }

            auto& frameData = it->second.frames[frameIndex];
            if (frameData.gpuBuffer.GetBuffer() != nullptr) {
                return frameData.gpuBuffer.GetBuffer()->GetDeviceAddress();
            }
        }

        return 0;
    }
}