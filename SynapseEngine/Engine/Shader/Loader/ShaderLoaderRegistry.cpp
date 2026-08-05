#include "ShaderLoaderRegistry.h"
#include <algorithm>

namespace Syn {
    void ShaderLoaderRegistry::Register(std::shared_ptr<IShaderLoader> loader, int priority) {
        for (const auto& ext : loader->GetSupportedExtensions()) {
            _loaders[ext].push_back({ priority, loader });
            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const ShaderLoaderEntry& a, const ShaderLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IShaderLoader*> ShaderLoaderRegistry::GetLoadersForExtension(const std::string& extension) {
        std::vector<IShaderLoader*> result;
        auto it = _loaders.find(extension);
        if (it != _loaders.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.loader.get());
            }
        }
        return result;
    }

    IShaderLoader* ShaderLoaderRegistry::GetLoaderForExtension(const std::string& extension) {
        auto it = _loaders.find(extension);
        if (it != _loaders.end() && !it->second.empty()) {
            return it->second.front().loader.get();
        }
        return nullptr;
    }
}