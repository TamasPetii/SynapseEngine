#pragma once
#include "EditorCore/Api/IFileSystemApi.h"

namespace Syn {
    class FileSystemApiImpl : public IFileSystemApi {
    public:
        std::vector<FileEntry> GetEntries(const std::string& directoryPath) const override;
        std::string GetParentPath(const std::string& path) const override;
        bool IsValidPath(const std::string& path) const override;
    };
}