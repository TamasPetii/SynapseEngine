#include "MaterialHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h" //Todo!
#include <algorithm>

namespace Syn {
    MaterialHierarchyViewModel::MaterialHierarchyViewModel(IMaterialApi* materialApi)
        : _materialApi(materialApi)
    {}

    void MaterialHierarchyViewModel::SyncWithEngine() {
        if (!_materialApi) return;

        uint64_t currentVersion = _materialApi->GetVersion();
        uint32_t currentSelection = _materialApi->GetSelectedMaterial();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_state.selectedMaterial != currentSelection) {
            _state.selectedMaterial = currentSelection;
        }
    }

    void MaterialHierarchyViewModel::Dispatch(const MaterialHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, MaterialSelectIntent>) {
                if (_materialApi) {
                    _materialApi->SetSelectedMaterial(arg.materialId);
                    _materialApi->ApplyMaterialToPreviewObjects(arg.materialId);
                }
            }
            else if constexpr (std::is_same_v<T, MaterialSetSearchQueryIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, MaterialRefreshIntent>) {
                _isDirty = true;
            }
            }, intent);
    }

    void MaterialHierarchyViewModel::RebuildList() {
        if (!_materialApi) return;

        _state.filteredNodes.clear();
        auto allMaterials = _materialApi->GetAllMaterials();

        std::string searchLower = _state.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto& mat : allMaterials) {
            std::string nameLower = mat.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (searchLower.empty() || nameLower.find(searchLower) != std::string::npos) {
                MaterialNode node;
                node.id = mat.id;
                node.name = mat.name;
                node.icon = SYN_ICON_BRUSH;
                _state.filteredNodes.push_back(node);
            }
        }
    }
}