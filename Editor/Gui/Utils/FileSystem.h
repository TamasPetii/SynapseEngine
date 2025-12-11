#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <memory>

struct FileEntry {
    std::string Path;
    std::string Name;
    std::string Extension;
    bool IsDirectory;
};

class IFileSystem {
public:
    virtual ~IFileSystem() = default;
    virtual std::vector<FileEntry> GetEntries(const std::string& path) = 0;
    virtual std::string GetParentPath(const std::string& path) = 0;
    virtual bool IsDirectory(const std::string& path) = 0;
    virtual std::string GetAbsolutePath(const std::string& path) = 0;
};

class NativeFileSystem : public IFileSystem {
public:
    std::vector<FileEntry> GetEntries(const std::string& directoryPath) override {
        std::vector<FileEntry> entries;
        try {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                FileEntry e;
                e.Path = entry.path().string();
                std::replace(e.Path.begin(), e.Path.end(), '\\', '/');
                e.Name = entry.path().filename().string();
                e.Extension = entry.path().extension().string();
                e.IsDirectory = entry.is_directory();

                if (e.IsDirectory) entries.insert(entries.begin(), e);
                else entries.push_back(e);
            }
        }
        catch (...) {}
        return entries;
    }

    std::string GetParentPath(const std::string& path) override {
        return std::filesystem::path(path).parent_path().string();
    }

    bool IsDirectory(const std::string& path) override {
        return std::filesystem::is_directory(path);
    }

    std::string GetAbsolutePath(const std::string& path) override {
        return std::filesystem::absolute(path).string();
    }
};