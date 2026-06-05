#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "HierarchyState.h"
#include "HierarchyIntent.h"
#include "EditorCore/Api/ITagApi.h"
#include "EditorCore/Api/IHierarchyApi.h"
#include "EditorCore/Api/ISelectionApi.h"
#include <unordered_set>

namespace Syn {
    class HierarchyViewModel : public IViewModel<HierarchyState, HierarchyIntent> {
    public:
        HierarchyViewModel(IHierarchyApi* hierarchyApi, ISelectionApi* selectionApi, ITagApi* tagApi);
        ~HierarchyViewModel() override = default;

        const HierarchyState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const HierarchyIntent& intent) override;

    private:
        void RebuildFlatList();
        bool TraverseAndFlatten(EntityID entity, int depth);
        void ExpandAllNodes(EntityID entity);
    private:
        IHierarchyApi* _hierarchyApi = nullptr;
        ISelectionApi* _selectionApi = nullptr;
		ITagApi* _tagApi = nullptr;

        HierarchyState _state;
        std::unordered_set<EntityID> _expandedNodes;
    };
}