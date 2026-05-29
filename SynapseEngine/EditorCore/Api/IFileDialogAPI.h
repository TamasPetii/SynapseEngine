#pragma once
#include <string>
#include <functional>

namespace Syn {

    struct FileDialogArgs {
        std::string Title;
        std::string Filters;
        std::string DefaultPath;
    };

    class IFileDialogAPI {
    public:
        virtual ~IFileDialogAPI() = default;

        virtual void OpenFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) = 0;
        virtual void SaveFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) = 0;
        virtual void Draw() = 0;
    };
}