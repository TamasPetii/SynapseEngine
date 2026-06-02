#pragma once
#include <string>
#include <vector>
#include "EditorCore/Types/FileEntry.h"

namespace Syn {
    struct ContentBrowserState {
        std::string currentPath;
        std::string selectedPath;
        std::vector<FileEntry> currentEntries;

        float thumbnailSize = 96.0f;
        bool isLoading = false;
    };
}