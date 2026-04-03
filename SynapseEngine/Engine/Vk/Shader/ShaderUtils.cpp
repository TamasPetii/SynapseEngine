#include "ShaderUtils.h"
#include <filesystem>
#include <string>

namespace Syn::Vk {
    VkShaderStageFlagBits ShaderUtils::GetStageFromExtension(const std::string& filepath) {
        std::filesystem::path path(filepath);
        std::string ext = path.extension().string();

        if (ext == ".vert") return VK_SHADER_STAGE_VERTEX_BIT;
        if (ext == ".frag") return VK_SHADER_STAGE_FRAGMENT_BIT;
        if (ext == ".geom") return VK_SHADER_STAGE_GEOMETRY_BIT;
        if (ext == ".tesc") return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (ext == ".tese") return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

        if (ext == ".comp") return VK_SHADER_STAGE_COMPUTE_BIT;

        if (ext == ".task") return VK_SHADER_STAGE_TASK_BIT_EXT;
        if (ext == ".mesh") return VK_SHADER_STAGE_MESH_BIT_EXT;

        if (ext == ".rgen") return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        if (ext == ".rany") return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        if (ext == ".rchit") return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        if (ext == ".rmiss") return VK_SHADER_STAGE_MISS_BIT_KHR;
        if (ext == ".rint") return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        if (ext == ".rcall") return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

        return (VkShaderStageFlagBits)0;
    }
}