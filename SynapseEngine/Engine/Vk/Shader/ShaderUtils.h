#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    class SYN_API ShaderUtils {
    public:
		static VkShaderStageFlagBits GetStageFromExtension(const std::string& filepath);
    };
}