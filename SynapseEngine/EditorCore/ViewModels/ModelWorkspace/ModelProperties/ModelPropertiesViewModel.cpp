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

#include "ModelPropertiesViewModel.h"

namespace Syn {
    ModelPropertiesViewModel::ModelPropertiesViewModel(IModelApi* modelApi)
        : _modelApi(modelApi) {}

    void ModelPropertiesViewModel::SyncWithEngine() {
        if (!_modelApi) return;

        auto selection = _modelApi->GetSelected();
        if (selection.first != _lastModelId || selection.second != _lastDescriptorIndex) {
            _lastModelId = selection.first;
            _lastDescriptorIndex = selection.second;
            UpdateState();
        }
    }

    void ModelPropertiesViewModel::Dispatch(const ModelPropertiesIntent& intent) {

    }

    void ModelPropertiesViewModel::UpdateState() {
        _state.hasSelection = (_lastModelId != 0xFFFFFFFF);
        _state.selectedModelId = _lastModelId;
        _state.selectedDescriptorIndex = _lastDescriptorIndex;

        if (!_state.hasSelection) return;

        const CpuModelData* cpuData = _modelApi->GetModelCpuData(_lastModelId);
        if (!cpuData) {
            _state.hasSelection = false;
            return;
        }

        auto models = _modelApi->GetAllModels();
        for (const auto& m : models) {
            if (m.id == _lastModelId) {
                _state.modelName = m.name;
                break;
            }
        }

        _state.globalVertexCount = cpuData->globalVertexCount;
        _state.globalIndexCount = cpuData->globalIndexCount;
        _state.globalMeshCount = cpuData->globalMeshCount;
        _state.globalAabbMin = cpuData->globalCollider.aabbMin;
        _state.globalAabbMax = cpuData->globalCollider.aabbMax;
        _state.globalCenter = cpuData->globalCollider.center;
        _state.globalRadius = cpuData->globalCollider.radius;

        if (_lastDescriptorIndex >= 0 && _lastDescriptorIndex < cpuData->meshNodeDescriptors.size()) {
            _state.isNodeSelected = true;
            const auto& desc = cpuData->meshNodeDescriptors[_lastDescriptorIndex];

            _state.nodeName = desc.name.empty() ? "Node_" + std::to_string(_lastDescriptorIndex) : desc.name;
            _state.nodeVertexCount = desc.vertexCount;
            _state.nodeIndexCount = desc.indexCount;
            _state.meshIndex = desc.meshIndex;
            _state.nodeIndex = desc.nodeIndex;
            _state.parentNodeIndex = desc.parentNodeIndex;

            if (desc.meshIndex != 0xFFFF) {
                uint32_t validMeshCounter = 0;

                for (int32_t i = 0; i < _lastDescriptorIndex; ++i) {
                    if (cpuData->meshNodeDescriptors[i].meshIndex != 0xFFFF) {
                        validMeshCounter++;
                    }
                }

                uint32_t lod0Index = validMeshCounter * 4;

                if (lod0Index < cpuData->meshletDrawDescriptors.size()) {
                    _state.nodeMeshletCount = cpuData->meshletDrawDescriptors[lod0Index].meshletCount;
                }
                if (lod0Index < cpuData->meshDescriptors.size()) {
                    _state.nodeMaterialIndex = cpuData->meshDescriptors[lod0Index].materialIndex;
                }
                if (validMeshCounter < cpuData->meshColliders.size()) 
                { 
                    _state.nodeAabbMin = cpuData->meshColliders[validMeshCounter].aabbMin;
                    _state.nodeAabbMax = cpuData->meshColliders[validMeshCounter].aabbMax;
                    _state.nodeRadius = cpuData->meshColliders[validMeshCounter].radius;
                }
            }
        }
        else {
            _state.isNodeSelected = false;
        }
    }
}