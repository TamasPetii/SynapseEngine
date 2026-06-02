#pragma once
#include <vector>
#include <string>
#include "EditorCore/Types/FileEntry.h"

namespace Syn 
{
    class IFileSystemAPI {
    public:
        virtual ~IFileSystemAPI() = default;

        virtual std::vector<FileEntry> GetEntries(const std::string& directoryPath) const = 0;
        virtual std::string GetParentPath(const std::string& path) const = 0;
        virtual bool IsValidPath(const std::string& path) const = 0;
    };
}