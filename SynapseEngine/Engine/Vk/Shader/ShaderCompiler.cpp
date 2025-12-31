#include "ShaderCompiler.h"
#include "Engine/SynMacro.h"
#include <fstream>
#include <filesystem>
#include <print>
#include "ShaderIncluder.h"
#include "Engine/Logger/Logger.h"

namespace Syn::Vk {
    std::vector<uint32_t> ShaderCompiler::Compile(const std::string& filepath, VkShaderStageFlagBits stage) {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetTargetSpirv(shaderc_spirv_version_1_6);
        options.SetIncluder(std::make_unique<ShaderIncluder>());

#ifdef SYN_DEBUG
        options.SetGenerateDebugInfo();
        options.SetOptimizationLevel(shaderc_optimization_level_zero);
#else
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif
        std::string source = LoadFile(filepath);
        shaderc_shader_kind kind = MapStageToKind(stage);

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, kind, filepath.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            std::string errorMsg = result.GetErrorMessage();
			Logger::Get().Dispatch(LogLevel::Error, std::format("SHADER COMPILE ERROR in {}:\n{}", filepath, errorMsg), "ShaderCompiler", 0);
            SYN_ASSERT(false, "Shader compilation failed!");
        }

        return { result.begin(), result.end() };
    }

    std::string ShaderCompiler::LoadFile(const std::string& filepath) {
        std::ifstream file(filepath);
        SYN_ASSERT(file.is_open(), ("Failed to open shader file: " + filepath).c_str());
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    shaderc_shader_kind ShaderCompiler::MapStageToKind(VkShaderStageFlagBits stage) {
        switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return shaderc_vertex_shader;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return shaderc_tess_control_shader;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return shaderc_tess_evaluation_shader;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return shaderc_geometry_shader;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return shaderc_fragment_shader;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return shaderc_compute_shader;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return shaderc_task_shader;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return shaderc_mesh_shader;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            return shaderc_raygen_shader;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            return shaderc_anyhit_shader;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            return shaderc_closesthit_shader;
        case VK_SHADER_STAGE_MISS_BIT_KHR:
            return shaderc_miss_shader;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            return shaderc_intersection_shader;
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return shaderc_callable_shader;
        default:
            return shaderc_glsl_infer_from_source;
        }
    }
}