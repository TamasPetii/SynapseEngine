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
#include "ModelHierarchyState.h"
#include "ModelHierarchyIntent.h"
#include "EditorCore/Api/IModelApi.h"
#include "EditorCore/Api/IPreviewApi.h"
#include <unordered_set>
#include <unordered_map>

namespace Syn {
    class ModelHierarchyViewModel : public IViewModel<ModelHierarchyState, ModelHierarchyIntent> {
    public:
        ModelHierarchyViewModel(IModelApi* modelApi, IPreviewApi* previewApi);
        ~ModelHierarchyViewModel() override = default;

        const ModelHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const ModelHierarchyIntent& intent) override;

    private:
        void RebuildModelList();
        void RebuildFlatList();
        bool TraverseAndFlatten(uint32_t modelId, int32_t descriptorIndex, uint16_t currentTransformIndex, int depth, const CpuModelData& cpuData, const std::unordered_map<uint16_t, std::vector<uint32_t>>& childMap, const std::string& nodeName);
        uint64_t GetNodeKey(uint32_t modelId, int32_t descriptorIndex) const;
    private:
        IModelApi* _modelApi = nullptr;
        IPreviewApi* _previewApi = nullptr;
        ModelHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
        std::unordered_set<uint64_t> _expandedNodes;
    };
}