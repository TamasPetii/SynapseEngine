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