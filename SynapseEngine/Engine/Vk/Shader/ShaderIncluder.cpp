#include "ShaderIncluder.h"
#include <filesystem>
#include <fstream>
#include <string>

namespace Syn::Vk {

    static std::string ReadFileContent(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    shaderc_include_result* ShaderIncluder::GetInclude(
        const char* requested_source,
        shaderc_include_type type,
        const char* requesting_source,
        size_t include_depth
    ) {
        std::filesystem::path requestPath = requested_source;
        std::filesystem::path currentDir = std::filesystem::path(requesting_source).parent_path();
        std::filesystem::path finalPath = currentDir / requestPath;

        if (!std::filesystem::exists(finalPath)) {
            finalPath = std::filesystem::path("Assets/Shaders") / requestPath;
        }

        if (!std::filesystem::exists(finalPath)) {
            finalPath = std::filesystem::path("Engine/Shaders") / requestPath;
        }

        std::string content = ReadFileContent(finalPath);

        if (content.empty() && !std::filesystem::exists(finalPath)) {
            std::string err = "Could not find include file: " + requestPath.string() +
                " (Searched relative, Assets/Shaders, and Engine/Shaders)";
            return new shaderc_include_result{ "", 0, err.c_str(), err.length(), nullptr };
        }

        auto* container = new std::pair<std::string, std::string>{ finalPath.string(), content };

        return new shaderc_include_result{
            container->first.c_str(),
            container->first.length(),
            container->second.c_str(),
            container->second.length(),
            container
        };
    }

    void ShaderIncluder::ReleaseInclude(shaderc_include_result* data) {
        delete static_cast<std::pair<std::string, std::string>*>(data->user_data);
        delete data;
    }
}