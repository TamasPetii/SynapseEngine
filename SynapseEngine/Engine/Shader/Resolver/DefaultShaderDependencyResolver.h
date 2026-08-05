#pragma once
#include "Engine/SynApi.h"
#include "IShaderDependencyResolver.h"

namespace Syn {
    class SYN_API DefaultShaderDependencyResolver : public IShaderDependencyResolver {
    public:
        DefaultShaderDependencyResolver() = default;
        ~DefaultShaderDependencyResolver() override = default;

        std::unordered_set<std::string> ResolveDependencies(const std::filesystem::path& sourcePath) override;
    private:
        void GatherDependencies(const std::filesystem::path& currentFile, std::unordered_set<std::string>& dependencies);
    };
}