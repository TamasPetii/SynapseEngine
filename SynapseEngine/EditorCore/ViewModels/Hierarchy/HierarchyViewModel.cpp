#include "HierarchyViewModel.h"
#include <algorithm>
#include <cctype>

namespace Syn {

    HierarchyViewModel::HierarchyViewModel(IHierarchyAPI* hierarchyApi, ISelectionAPI* selectionApi)
        : _hierarchyApi(hierarchyApi), _selectionApi(selectionApi)
    {      
    }

    void HierarchyViewModel::SyncWithEngine() {
        if (!_selectionApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (_state.selectedEntity != activeEntity) {
            _state.selectedEntity = activeEntity;
        }

        RebuildFlatList();
    }

    void HierarchyViewModel::Dispatch(const HierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, SelectEntityIntent>) {
                _selectionApi->SetSelectedEntity(arg.entity);
                _state.selectedEntity = arg.entity;
            }
            else if constexpr (std::is_same_v<T, ToggleExpandIntent>) {
                if (arg.expand) _expandedNodes.insert(arg.entity);
                else _expandedNodes.erase(arg.entity);

                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, ToggleVisibilityIntent>) {
                _hierarchyApi->SetEntityVisibility(arg.entity, arg.visible);
                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, ReparentEntityIntent>) {
                _hierarchyApi->SetParent(arg.child, arg.newParent);
                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, CreateEntityIntent>) {
                EntityID newEnt = _hierarchyApi->CreateEntity(arg.name, arg.parent);
                if (arg.parent != NULL_ENTITY) {
                    _expandedNodes.insert(arg.parent);
                }
                _selectionApi->SetSelectedEntity(newEnt);

                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, DestroyEntityIntent>) {
                _hierarchyApi->DestroyEntity(arg.entity);
                if (_state.selectedEntity == arg.entity) {
                    _selectionApi->SetSelectedEntity(NULL_ENTITY);
                }

                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, RefreshHierarchyIntent>) {
                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, SetSearchQueryIntent>) {
                _state.searchQuery = arg.query;
                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, ExpandAllIntent>) {
                for (EntityID root : _hierarchyApi->GetRootEntities()) {
                    ExpandAllNodes(root);
                }
                RebuildFlatList();
            }
            else if constexpr (std::is_same_v<T, CollapseAllIntent>) {
                _expandedNodes.clear();
                RebuildFlatList();
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
        std::string name = _hierarchyApi->GetEntityName(entity);

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
            _hierarchyApi->IsEntityVisible(entity)
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