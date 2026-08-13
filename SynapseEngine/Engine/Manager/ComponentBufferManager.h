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
#include "Engine/Utils/WindowedBuffer.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Syn
{
    enum class SYN_API ComponentMemoryType {
        Persistent,
        GpuOnly
    };

    struct SYN_API ComponentBufferView
    {
        Vk::Buffer* buffer;
        std::span<uint32_t> versions;
    };

    class SYN_API ComponentBufferManager
    {
    public:
        ComponentBufferManager(uint32_t frameCount);

        void RegisterBuffer(const std::string& name, uint32_t elementSize, std::function<uint32_t()> sizeCallback, std::function<bool()> readyCallback, ComponentMemoryType memoryType = ComponentMemoryType::Persistent);
        void Update(uint32_t frameIndex);

        ComponentBufferView GetComponentBuffer(const std::string& name, uint32_t frameIndex);
        uint64_t GetBufferAddr(const std::string& name, uint32_t frameIndex);
    private:
        struct FrameData
        {
            WindowedBuffer gpuBuffer;
            std::vector<uint32_t> versions;
        };

        struct ComponentBufferState
        {
            std::vector<FrameData> frames;
            std::function<uint32_t()> sizeCallback;
            std::function<bool()> readyCallback;
            ComponentMemoryType memoryType;
        };

        uint32_t _frameCount;
        std::unordered_map<std::string, ComponentBufferState> _buffers;
    };
}