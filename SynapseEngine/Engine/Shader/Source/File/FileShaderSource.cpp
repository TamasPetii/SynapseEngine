#include "FileShaderSource.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    FileShaderSource::FileShaderSource(const std::filesystem::path& path, IShaderLoader* loader, VkShaderStageFlagBits stage, std::vector<std::string> defines)
        : _path(path), _loader(loader), _stage(stage), _defines(std::move(defines))
    {}

    std::optional<RawShader> FileShaderSource::Produce() {
        if (!_loader) {
            Error("No loader provided for FileShaderSource: {}", _path.string());
            return std::nullopt;
        }

        return _loader->LoadFile(_path, _stage, _defines);
    }
}