#pragma once
#include "IApi.h"
#include <vector>
#include <string>
#include "EditorCore/Types/FileEntry.h"

namespace Syn 
{
    class IFileSystemApi : public IApi {
    public:
        virtual ~IFileSystemApi() = default;

        virtual std::vector<FileEntry> GetEntries(const std::string& directoryPath) const = 0;
        virtual std::string GetParentPath(const std::string& path) const = 0;
        virtual bool IsValidPath(const std::string& path) const = 0;
    };
}