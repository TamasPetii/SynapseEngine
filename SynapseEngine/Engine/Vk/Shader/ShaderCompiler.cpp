#include "ShaderCompiler.h"
#include "Engine/SynMacro.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <print>
#include <regex>
#include <unordered_set>
#include "ShaderIncluder.h"
#include "Engine/Logger/Logger.h"

namespace Syn::Vk {
    static void GatherDependencies(const std::filesystem::path& currentFile, std::unordered_set<std::string>& dependencies) {
        if (!std::filesystem::exists(currentFile)) return;

        std::string pathStr = currentFile.lexically_normal().string();
        if (dependencies.contains(pathStr)) return;
        dependencies.insert(pathStr);

        std::ifstream file(currentFile);
        if (!file.is_open()) return;

        std::string line;
        std::regex includeRegex(R"x(^[ \t]*#[ \t]*include[ \t]+["<](.*)[">])x");
        std::smatch match;

        while (std::getline(file, line)) {
            if (std::regex_search(line, match, includeRegex)) {
                if (match.size() == 2) {
                    std::filesystem::path includePath = match[1].str();
                    std::filesystem::path currentDir = currentFile.parent_path();

                    std::filesystem::path finalPath = (currentDir / includePath).lexically_normal();

                    if (!std::filesystem::exists(finalPath)) {
                        finalPath = (std::filesystem::path("Assets/Shaders") / includePath).lexically_normal();
                    }

                    if (!std::filesystem::exists(finalPath)) {
                        finalPath = (std::filesystem::path("Engine/Shaders") / includePath).lexically_normal();
                    }

                    GatherDependencies(finalPath, dependencies);
                }
            }
        }
    }

    std::vector<uint32_t> ShaderCompiler::Compile(const std::string& filepath, VkShaderStageFlagBits stage) {
        namespace fs = std::filesystem;

        fs::path sourcePath(filepath);

        const char* appDataPath = std::getenv("APPDATA");
        fs::path baseDir = appDataPath ? appDataPath : ".";
        fs::path cacheDir = baseDir / "Synapse" / "Shaders";

        if (!fs::exists(cacheDir)) {
            fs::create_directories(cacheDir);
        }

        std::string cacheFilename = filepath;
        std::replace(cacheFilename.begin(), cacheFilename.end(), '/', '_');
        std::replace(cacheFilename.begin(), cacheFilename.end(), '\\', '_');
        std::replace(cacheFilename.begin(), cacheFilename.end(), ':', '_');
        
        #if defined(SYN_DEBUG)
                cacheFilename += "_debug";
        #elif defined(SYN_RELEASE)
                cacheFilename += "_release";
        #elif defined(SYN_DIST)
                cacheFilename += "_dist";
        #else
            cacheFilename += "_unknown";
        #endif
        
        fs::path cachePath = cacheDir / (cacheFilename + ".spv");

        bool needsCompile = true;

        if (fs::exists(cachePath) && fs::exists(sourcePath)) {
            auto cacheTime = fs::last_write_time(cachePath);
            needsCompile = false;

            std::unordered_set<std::string> dependencies;
            GatherDependencies(sourcePath, dependencies);

            for (const auto& dep : dependencies) {
                if (fs::exists(dep)) {
                    auto depTime = fs::last_write_time(dep);
                    if (depTime > cacheTime) {
                        needsCompile = true;
                        break;
                    }
                }
            }
        }

        if (!needsCompile) {
            std::ifstream file(cachePath, std::ios::ate | std::ios::binary);
            if (file.is_open()) {
                size_t fileSize = (size_t)file.tellg();
                std::vector<uint32_t> spirv(fileSize / sizeof(uint32_t));

                file.seekg(0);
                file.read(reinterpret_cast<char*>(spirv.data()), fileSize);
                file.close();

                Info("Loaded cached shader: {}", filepath);
                return spirv;
            }
        }

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
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
            Error("Shader compile Error in {}:\n{}", filepath, errorMsg);
            SYN_ASSERT(false, "Shader compilation failed!");
        }

        std::vector<uint32_t> spirv(result.begin(), result.end());

        std::ofstream outFile(cachePath, std::ios::out | std::ios::binary);
        if (outFile.is_open()) {
            outFile.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
            outFile.close();
        }

        Info("Compiled and cached shader: {}", filepath);
        return spirv;
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