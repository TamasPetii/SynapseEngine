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
                node.path = tex.path;
                node.handle = tex.handle;
                node.icon = SYN_ICON_IMAGE;
                _state.filteredNodes.push_back(node);
            }
        }
    }
}