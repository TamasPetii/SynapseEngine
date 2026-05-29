#pragma once
#include "EditorCore/Api/IFileDialogAPI.h"
#include "../External/ImGuiFileDialog/ImGuiFileDialog.h"

namespace Syn 
{
    class ImGuiFileDialogImpl : public IFileDialogAPI {
    public:
        void OpenFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) override {
            _onResult = onResult;
            IGFD::FileDialogConfig config;
            config.path = args.DefaultPath;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", args.Title.c_str(), args.Filters.c_str(), config);
        }

        void SaveFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) override {
            _onResult = onResult;
            IGFD::FileDialogConfig config;
            config.path = args.DefaultPath;
            config.flags = ImGuiFileDialogFlags_Modal;
            ImGuiFileDialog::Instance()->OpenDialog("SaveFileDlgKey", args.Title.c_str(), args.Filters.c_str(), config);
        }

        void Draw() override 
        {
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    if (_onResult) _onResult(filePathName);
                }
                ImGuiFileDialog::Instance()->Close();
                _onResult = nullptr;
            }

            if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    if (_onResult) _onResult(filePathName);
                }
                ImGuiFileDialog::Instance()->Close();
                _onResult = nullptr;
            }
        }
    private:
        std::function<void(const std::string&)> _onResult;
    };
}