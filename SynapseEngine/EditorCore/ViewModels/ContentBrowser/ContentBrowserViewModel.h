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