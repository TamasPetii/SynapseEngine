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

#pragma once
#include "EditorCore/Api/IFileDialogApi.h"
#include "../External/ImGuiFileDialog/ImGuiFileDialog.h"

namespace Syn 
{
    class ImGuiFileDialogImpl : public IFileDialogApi {
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