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
#include "Buffer.h"

namespace Syn::Vk {

    class SYN_API BufferFactory {
    public:
        static void Allocate(Buffer* buffer);
        static std::unique_ptr<Buffer> CreateGpu(VkDeviceSize size,VkBufferUsageFlags usage);
        static std::unique_ptr<Buffer> CreatePersistent(VkDeviceSize size,VkBufferUsageFlags usage);
        static std::unique_ptr<Buffer> CreateStaging(VkDeviceSize size);
        static std::unique_ptr<Buffer> CreateVideoBitstream(VkDeviceSize size, const void* pNextExtension);
        static std::unique_ptr<Buffer> Create(const BufferConfig& config);
    };
}