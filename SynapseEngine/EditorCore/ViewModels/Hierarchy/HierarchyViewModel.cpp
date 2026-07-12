#include "HierarchyViewModel.h"
#include <algorithm>
#include <cctype>

namespace Syn {

    HierarchyViewModel::HierarchyViewModel(IHierarchyApi* hierarchyApi, ISelectionApi* selectionApi, ITagApi* tagApi)
        : _hierarchyApi(hierarchyApi), _selectionApi(selectionApi), _tagApi(tagApi)
    {      
    }

    void HierarchyViewModel::SyncWithEngine() {
        if (!_selectionApi || !_hierarchyApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (_state.selectedEntity != activeEntity) {
            _state.selectedEntity = activeEntity;
        }

        uint64_t currentEngineVersion = _hierarchyApi->GetVersion();
        if (_lastEngineVersion != currentEngineVersion) {
            _isDirty = true;
            _lastEngineVersion = currentEngineVersion;
        }

        if (_isDirty) {
            RebuildFlatList();
            _isDirty = false;
        }
    }

    void HierarchyViewModel::Dispatch(const HierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, HierarchySelectEntityIntent>) {
                _selectionApi->SetSelectedEntity(arg.entity);
                _state.selectedEntity = arg.entity;
            }
            else if constexpr (std::is_same_v<T, HierarchyToggleExpandIntent>) {
                if (arg.expand) _expandedNodes.insert(arg.entity);
                else _expandedNodes.erase(arg.entity);
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, HierarchyToggleVisibilityIntent>) {
                _tagApi->SetEntityEnabled(arg.entity, !_tagApi->IsEntityEnabled(arg.entity));
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, HierarchyReparentEntityIntent>) {
                _hierarchyApi->SetParent(arg.child, arg.newParent);
            }
            else if constexpr (std::is_same_v<T, HierarchyCreateEntityIntent>) {
                EntityID newEnt = _hierarchyApi->CreateEntity(arg.type, arg.parent);
                if (arg.parent != NULL_ENTITY) {
                    _expandedNodes.insert(arg.parent);
                }
                _selectionApi->SetSelectedEntity(newEnt);
            }
            else if constexpr (std::is_same_v<T, HierarchyCopyEntityIntent>) {
                EntityID newEnt = _hierarchyApi->CopyEntity(arg.entity, _hierarchyApi->GetParent(arg.entity));
                _selectionApi->SetSelectedEntity(newEnt);
            }
            else if constexpr (std::is_same_v<T, HierarchyFullCopyEntityIntent>) {
                EntityID newEnt = _hierarchyApi->FullCopyEntity(arg.entity, _hierarchyApi->GetParent(arg.entity));
                _selectionApi->SetSelectedEntity(newEnt);
            }
            else if constexpr (std::is_same_v<T, HierarchyDestroyEntityIntent>) {
                _hierarchyApi->DestroyEntityRecursive(arg.entity);
                if (_state.selectedEntity == arg.entity) {
                    _selectionApi->SetSelectedEntity(NULL_ENTITY);
                }
            }
            else if constexpr (std::is_same_v<T, HierarchyDestroyKeepChildrenIntent>) {
                _hierarchyApi->DestroyEntityKeepChildren(arg.entity);
                if (_state.selectedEntity == arg.entity) {
                    _selectionApi->SetSelectedEntity(NULL_ENTITY);
                }
            }
            else if constexpr (std::is_same_v<T, HierarchyRefreshHierarchyIntent>) {
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, HierarchySetSearchQueryIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, HierarchyExpandAllIntent>) {
                for (EntityID root : _hierarchyApi->GetRootEntities()) {
                    ExpandAllNodes(root);
                }
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, HierarchyCollapseAllIntent>) {
                _expandedNodes.clear();
                _isDirty = true;
            }
            }, intent);
    }

    void HierarchyViewModel::ExpandAllNodes(EntityID entity) {
        if (_hierarchyApi->HasChildren(entity)) {
            _expandedNodes.insert(entity);
            for (EntityID child : _hierarchyApi->GetChildren(entity)) {
                ExpandAllNodes(child);
            }
        }
    }

    void HierarchyViewModel::RebuildFlatList() {
        if (!_hierarchyApi) return;

        _state.flatNodes.clear();

        auto rootEntities = _hierarchyApi->GetRootEntities();

        for (EntityID root : rootEntities) {
            TraverseAndFlatten(root, 0);
        }
    }

    bool HierarchyViewModel::TraverseAndFlatten(EntityID entity, int depth) {
        std::string name = _tagApi->GetEntityName(entity);

        bool matchesSearch = _state.searchQuery.empty() ||
            std::search(name.begin(), name.end(), _state.searchQuery.begin(), _state.searchQuery.end(),
                [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != name.end();

        bool hasChildren = _hierarchyApi->HasChildren(entity);
        bool isExpanded = _expandedNodes.contains(entity) || !_state.searchQuery.empty();

        size_t nodeIndex = _state.flatNodes.size();

        _state.flatNodes.push_back({
            entity,
            name,
            _hierarchyApi->GetEntityIcon(entity),
            depth,
            hasChildren,
            isExpanded,
            _tagApi->IsEntityEnabled(entity)
            });

        bool anyChildMatches = false;

        if (isExpanded && hasChildren) {
            auto children = _hierarchyApi->GetChildren(entity);
            for (EntityID child : children) {
                if (TraverseAndFlatten(child, depth + 1)) {
                    anyChildMatches = true;
                }
            }
        }

        if (!_state.searchQuery.empty() && !matchesSearch && !anyChildMatches) {
            _state.flatNodes.resize(nodeIndex);
            return false;
        }

        return true;
    }
}