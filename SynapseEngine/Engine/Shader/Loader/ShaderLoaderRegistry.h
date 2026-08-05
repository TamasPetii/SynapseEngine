#pragma once
#include "Engine/SynApi.h"
#include "IShaderLoaderRegistry.h"
#include <unordered_map>

namespace Syn {
    class SYN_API ShaderLoaderRegistry : public IShaderLoaderRegistry {
    public:
        void Register(std::shared_ptr<IShaderLoader> loader, int priority = 0) override;
        std::vector<IShaderLoader*> GetLoadersForExtension(const std::string& extension) override;
        IShaderLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<ShaderLoaderEntry>> _loaders;
    };
}