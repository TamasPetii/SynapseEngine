#include "EditorApiImpl.h"
#include <filesystem>
#include <algorithm>
#include <system_error>

namespace Syn {

    std::vector<FileEntry> EditorApiImpl::GetEntries(const std::string& directoryPath) const {
        std::vector<FileEntry> entries;
        std::error_code ec;

        if (!std::filesystem::exists(directoryPath, ec) || !std::filesystem::is_directory(directoryPath, ec)) {
            return entries;
        }

        for (const auto& entry : std::filesystem::directory_iterator(directoryPath, ec)) {
            if (ec) continue;

            FileEntry fileEntry;
            const auto& path = entry.path();

            fileEntry.name = path.filename().string();
            fileEntry.path = path.generic_string();
            fileEntry.extension = path.extension().string();
            fileEntry.isDirectory = entry.is_directory(ec);

            entries.push_back(fileEntry);
        }

        std::sort(entries.begin(), entries.end(), [](const FileEntry& a, const FileEntry& b) {
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory > b.isDirectory;
            }
            return a.name < b.name;
            });

        return entries;
    }

    std::string EditorApiImpl::GetParentPath(const std::string& path) const {
        std::filesystem::path p(path);
        return p.parent_path().generic_string();
    }

    bool EditorApiImpl::IsValidPath(const std::string& path) const {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
    }

}