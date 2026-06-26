#include "TextureHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h"
#include <algorithm>

namespace Syn {
    TextureHierarchyViewModel::TextureHierarchyViewModel(ITextureApi* textureApi)
        : _textureApi(textureApi)
    {}

    void TextureHierarchyViewModel::SyncWithEngine() {
        if (!_textureApi) return;

        uint64_t currentVersion = _textureApi->GetVersion();
        uint32_t currentSelection = _textureApi->GetSelectedTexture();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_state.selectedTexture != currentSelection) {
            _state.selectedTexture = currentSelection;
        }
    }

    void TextureHierarchyViewModel::Dispatch(const TextureHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, TextureSelectIntent>) {
                if (_textureApi) {
                    _textureApi->SetSelectedTexture(arg.textureId);
                }
            }
            else if constexpr (std::is_same_v<T, TextureSetSearchQueryIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, TextureRefreshIntent>) {
                _isDirty = true;
            }
            }, intent);
    }

    void TextureHierarchyViewModel::RebuildList() {
        if (!_textureApi) return;

        _state.filteredNodes.clear();
        auto allTextures = _textureApi->GetAllTextures();

        std::string searchLower = _state.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto& tex : allTextures) {
            std::string nameLower = tex.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (searchLower.empty() || nameLower.find(searchLower) != std::string::npos) {
                TextureNode node;
                node.id = tex.id;
                node.name = tex.name;
                node.icon = SYN_ICON_IMAGE;
                _state.filteredNodes.push_back(node);
            }
        }
    }
}