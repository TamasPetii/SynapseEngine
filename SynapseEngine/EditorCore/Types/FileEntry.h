#pragma once
#include <string>

namespace Syn {
    struct FileEntry {
        std::string name;
        std::string path;
        std::string extension;
        bool isDirectory = false;
    };
}