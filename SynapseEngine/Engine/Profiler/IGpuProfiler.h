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
#include "IProfiler.h"
#include <vulkan/vulkan.h>

namespace Syn {

    class SYN_API IGpuProfiler : public IProfiler {
    public:
        virtual void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
        virtual void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t measurementIndex) = 0;
        virtual uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) = 0;
    };
}