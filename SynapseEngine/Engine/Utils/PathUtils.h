#pragma once
#include "Engine/SynApi.h"
#include <filesystem>
#include <string>

#ifndef SYN_PROJECT_ROOT
#define SYN_PROJECT_ROOT "."
#endif

namespace Syn {
    class SYN_API PathUtils {
    public:
        static inline std::filesystem::path GetAbsolutePath(const std::filesystem::path& path) {
            if (path.is_absolute()) return path.lexically_normal();
            std::filesystem::path root(SYN_PROJECT_ROOT);
            return (root / path).lexically_normal();
        }

        static inline std::string GetAbsolutePathString(const std::string& path) {
            return GetAbsolutePath(path).string();
        }
    };
}