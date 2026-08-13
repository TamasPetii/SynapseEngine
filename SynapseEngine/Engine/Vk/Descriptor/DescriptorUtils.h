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
#include "../VkCommon.h"
#include <vector>
#include <utility>
#include <span>

namespace Syn::Vk 
{
    class DescriptorBuffer;

    class SYN_API DescriptorUtils {
    public:
        static void Cleanup();
        static VkDescriptorSetLayout GetEmptyDescriptorSetLayout(bool useDescriptorBuffers);
        static void BindMultipleBuffer(
            VkCommandBuffer cmd,
            VkPipelineLayout pipelineLayout,
            VkPipelineBindPoint bindPoint,
            std::span<const std::pair<uint32_t, DescriptorBuffer*>> sets
        );
    private:
        static VkDescriptorSetLayout _emptyBufferLayout;
        static VkDescriptorSetLayout _emptyStandardLayout;
     };
}