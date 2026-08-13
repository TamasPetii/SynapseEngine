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
#include "AnimationHierarchyState.h"
#include "AnimationHierarchyIntent.h"
#include "EditorCore/Api/IAnimationApi.h"
#include "EditorCore/Api/IModelApi.h"
#include "EditorCore/Api/IPreviewApi.h"
#include <unordered_set>
#include <unordered_map>

namespace Syn {
    class AnimationHierarchyViewModel : public IViewModel<AnimationHierarchyState, AnimationHierarchyIntent> {
    public:
        AnimationHierarchyViewModel(IAnimationApi* animationApi, IModelApi* modelApi, IPreviewApi* previewApi);
        ~AnimationHierarchyViewModel() override = default;

        const AnimationHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const AnimationHierarchyIntent& intent) override;

    private:
        void RebuildAnimationList();
        void RebuildFlatList();
        bool TraverseAndFlatten(uint32_t animationId, int32_t descriptorIndex, uint16_t currentTransformIndex, int depth, const CpuModelData& cpuData, const std::unordered_map<uint16_t, std::vector<uint32_t>>& childMap, const std::string& nodeName, const CpuAnimationData& animData);
        uint64_t GetNodeKey(uint32_t animationId, int32_t descriptorIndex) const;
    private:
        IAnimationApi* _animationApi = nullptr;
        IModelApi* _modelApi = nullptr;
        IPreviewApi* _previewApi = nullptr;

        AnimationHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
        std::unordered_set<uint64_t> _expandedNodes;
    };
}