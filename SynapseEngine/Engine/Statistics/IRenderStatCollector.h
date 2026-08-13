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
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Syn
{
    struct SYN_API RenderPassStats {
        std::string groupName;
        std::string passName;

        uint64_t inputAssemblyVertices = 0;
        uint64_t inputAssemblyPrimitives = 0;
        uint64_t vertexShaderInvocations = 0;
        uint64_t clippingInvocations = 0;
        uint64_t clippingPrimitives = 0;
        uint64_t fragmentShaderInvocations = 0;
        uint64_t taskShaderInvocations = 0;
        uint64_t meshShaderInvocations = 0;
    };

    class SYN_API IRenderStatCollector {
    public:
        virtual ~IRenderStatCollector() = default;

        virtual void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) = 0;
        virtual uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) = 0;
        virtual void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t queryIndex) = 0;

        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::vector<RenderPassStats>& GetStats(uint32_t frameIndex) const = 0;
    };
}