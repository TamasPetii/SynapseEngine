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

		bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
    };
}