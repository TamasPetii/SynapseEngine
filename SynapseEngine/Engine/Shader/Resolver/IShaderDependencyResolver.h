#pragma once
#include "Engine/SynApi.h"
#include <filesystem>
#include <unordered_set>
#include <string>

namespace Syn {
    class SYN_API IShaderDependencyResolver {
    public:
        virtual ~IShaderDependencyResolver() = default;
        virtual std::unordered_set<std::string> ResolveDependencies(const std::filesystem::path& sourcePath) = 0;
    };
}