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
#include "EditorCore/ViewModels/IViewModel.h"
#include "ContentBrowserState.h"
#include "ContentBrowserIntent.h"
#include "EditorCore/Api/IFileSystemApi.h"

namespace Syn {
    class ContentBrowserViewModel : public IViewModel<ContentBrowserState, ContentBrowserIntent> {
    public:
        ContentBrowserViewModel(IFileSystemApi* fileSystemApi, const std::string& initialPath)
            : _fileSystemApi(fileSystemApi)
        {
            _state.currentPath = initialPath;
            LoadCurrentDirectory();
        }

        const ContentBrowserState& GetState() const override { return _state; }

        void SyncWithEngine() override {
        }

        void Dispatch(const ContentBrowserIntent& intent) override {
            std::visit([this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, ChangeDirectoryIntent>) {
                    if (_fileSystemApi->IsValidPath(arg.newPath)) {
                        _state.currentPath = arg.newPath;
                        _state.selectedPath.clear();
                        LoadCurrentDirectory();
                    }
                }
                else if constexpr (std::is_same_v<T, SelectEntryIntent>) {
                    _state.selectedPath = arg.path;
                }
                else if constexpr (std::is_same_v<T, SetThumbnailSizeIntent>) {
                    _state.thumbnailSize = arg.newSize;
                }
                else if constexpr (std::is_same_v<T, RefreshDirectoryIntent>) {
                    LoadCurrentDirectory();
                }
                }, intent);
        }

    private:
        void LoadCurrentDirectory() {
            if (!_fileSystemApi) return;

            _state.isLoading = true;
            _state.currentEntries = _fileSystemApi->GetEntries(_state.currentPath);
            _state.isLoading = false;
        }

    private:
        IFileSystemApi* _fileSystemApi = nullptr;
        ContentBrowserState _state;
    };
}