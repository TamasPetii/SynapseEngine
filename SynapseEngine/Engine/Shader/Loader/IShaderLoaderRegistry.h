#pragma once
#include "Engine/SynApi.h"
#include "IShaderLoader.h"
#include <memory>
#include <string>
#include <vector>

namespace Syn {
    struct SYN_API ShaderLoaderEntry {
        int priority;
        std::shared_ptr<IShaderLoader> loader;
    };

    class SYN_API IShaderLoaderRegistry {
    public:
        virtual ~IShaderLoaderRegistry() = default;
        virtual void Register(std::shared_ptr<IShaderLoader> loader, int priority = 0) = 0;
        virtual IShaderLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IShaderLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}