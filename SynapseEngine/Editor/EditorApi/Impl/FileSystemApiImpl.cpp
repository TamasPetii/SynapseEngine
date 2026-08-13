// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "FileSystemApiImpl.h"
#include <filesystem>
#include <algorithm>
#include <system_error>

namespace Syn {
    std::vector<FileEntry> FileSystemApiImpl::GetEntries(const std::string& directoryPath) const {
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
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.name < b.name;
        });

        return entries;
    }

    std::string FileSystemApiImpl::GetParentPath(const std::string& path) const {
        return std::filesystem::path(path).parent_path().generic_string();
    }

    bool FileSystemApiImpl::IsValidPath(const std::string& path) const {
        std::error_code ec;
        return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec);
    }
}