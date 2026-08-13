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
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <algorithm>

namespace Syn
{
    class SYN_API WindowedBuffer
    {
    public:
        WindowedBuffer(const Vk::BufferConfig& baseConfig, uint32_t elementSize, uint32_t upWindow = 256, uint32_t downWindow = 512);
        std::pair<bool, std::shared_ptr<Vk::Buffer>> UpdateCapacity(uint64_t requiredElements);

        Vk::Buffer* GetBuffer() const { return _buffer.get(); }
        uint64_t GetCapacity() const { return _capacity; }
        size_t GetElementSize() const { return _elementSize; }
    private:
        Vk::BufferConfig _baseConfig;
        size_t _elementSize;
        uint32_t _upWindow;
        uint32_t _downWindow;
        uint64_t _capacity = 0;
        std::shared_ptr<Vk::Buffer> _buffer;
    };
}