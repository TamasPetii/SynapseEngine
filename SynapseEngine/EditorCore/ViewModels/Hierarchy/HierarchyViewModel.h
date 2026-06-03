#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "HierarchyState.h"
#include "HierarchyIntent.h"
#include "EditorCore/API/ITagAPI.h"
#include "EditorCore/Api/IHierarchyAPI.h"
#include "EditorCore/Api/ISelectionAPI.h"
#include <unordered_set>

namespace Syn {
    class HierarchyViewModel : public IViewModel<HierarchyState, HierarchyIntent> {
    public:
        HierarchyViewModel(IHierarchyAPI* hierarchyApi, ISelectionAPI* selectionApi, ITagAPI* tagApi);
        ~HierarchyViewModel() override = default;

        const HierarchyState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const HierarchyIntent& intent) override;

    private:
        void RebuildFlatList();
        bool TraverseAndFlatten(EntityID entity, int depth);
        void ExpandAllNodes(EntityID entity);
    private:
        IHierarchyAPI* _hierarchyApi = nullptr;
        ISelectionAPI* _selectionApi = nullptr;
		ITagAPI* _tagApi = nullptr;

        HierarchyState _state;
        std::unordered_set<EntityID> _expandedNodes;
    };
}