#pragma once
#include "Engine/Shader/Source/IShaderSource.h"
#include "Engine/Shader/Loader/IShaderLoader.h"
#include <filesystem>
#include <vector>
#include <string>

namespace Syn {
    class SYN_API FileShaderSource : public IShaderSource {
    public:
        FileShaderSource(const std::filesystem::path& path, IShaderLoader* loader, VkShaderStageFlagBits stage, std::vector<std::string> defines = {});
        std::optional<RawShader> Produce() override;
    private:
        std::filesystem::path _path;
        IShaderLoader* _loader;
        VkShaderStageFlagBits _stage;
        std::vector<std::string> _defines;
    };
}