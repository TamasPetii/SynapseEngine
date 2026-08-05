#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/VkCommon.h"
#include "Engine/Shader/Data/Common/ShaderLanguageType.h"
#include <string>
#include <vector>

namespace Syn {

    struct SYN_API RawShader {
        std::string sourceCode;
        std::string identifier;
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        ShaderLanguage language = ShaderLanguage::GLSL;
        std::vector<std::string> defines;
    };
}