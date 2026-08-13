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

#include "DefaultGpuShaderUploader.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    ShaderUploadResult DefaultGpuShaderUploader::Upload(const GpuShaderData& data) {
        ShaderUploadResult result;

        if (data.spirv.empty()) {
            Error("Cannot upload shader, SPIR-V data is empty.");
            return result;
        }

        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkShaderCreateInfoEXT createInfo = { VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT };

        createInfo.stage = data.stage;
        createInfo.codeType = VK_SHADER_CODE_TYPE_SPIRV_EXT;
        createInfo.pCode = data.spirv.data();
        createInfo.codeSize = data.spirv.size() * sizeof(uint32_t);
        createInfo.pName = "main";
        createInfo.flags = 0;

        SYN_VK_ASSERT_MSG(vkCreateShadersEXT(device->Handle(), 1, &createInfo, nullptr, &result.handle), "Failed to create Shader Object EXT!");

        return result;
    }
}