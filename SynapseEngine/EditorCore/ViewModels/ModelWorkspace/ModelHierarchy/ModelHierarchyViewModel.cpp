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

#include "ModelHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h"
#include <algorithm>
#include <cctype>

namespace Syn {
    ModelHierarchyViewModel::ModelHierarchyViewModel(IModelApi* modelApi, IPreviewApi* previewApi)
        : _modelApi(modelApi), _previewApi(previewApi)
    {}

    uint64_t ModelHierarchyViewModel::GetNodeKey(uint32_t modelId, int32_t descriptorIndex) const {
        return (static_cast<uint64_t>(modelId) << 32) | static_cast<uint32_t>(descriptorIndex);
    }

    void ModelHierarchyViewModel::SyncWithEngine() {
        if (!_modelApi) return;

        uint64_t currentVersion = _modelApi->GetVersion();
        auto selection = _modelApi->GetSelected();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildModelList();
            RebuildFlatList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_state.selectedModelId != selection.first || _state.selectedDescriptorIndex != selection.second) {
            _state.selectedModelId = selection.first;
            _state.selectedDescriptorIndex = selection.second;
        }

        if (_previewApi) {
            _state.atlasHandle = _previewApi->GetAtlasHandle();

            for (auto& node : _state.filteredModels) {
                if (!node.hasPreview) {
                    if (_previewApi->GetPreviewUVs(PreviewResourceType::Model, node.id, node.uv0, node.uv1)) {
                        node.hasPreview = true;
                    }
                }
            }
        }
    }

    void ModelHierarchyViewModel::Dispatch(const ModelHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, ModelHierarchySelectIntent>) {
                if (_modelApi) {
                    _modelApi->SetSelected(arg.modelId, arg.descriptorIndex);
                    _modelApi->ApplyModelToPreviewObject(arg.modelId);
                }

                if (_state.selectedModelId != arg.modelId) {
                    _state.selectedModelId = arg.modelId;
                    _state.selectedDescriptorIndex = arg.descriptorIndex;
                    _isDirty = true;
                }
                else {
                    _state.selectedDescriptorIndex = arg.descriptorIndex;
                }
            }
            else if constexpr (std::is_same_v<T, ModelHierarchyToggleExpandIntent>) {
                uint64_t key = GetNodeKey(arg.modelId, arg.descriptorIndex);
                if (arg.expand) _expandedNodes.insert(key);
                else _expandedNodes.erase(key);
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, ModelHierarchySetSearchIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, ModelHierarchyRefreshIntent>) {
                _isDirty = true;
            }
            }, intent);
    }

    void ModelHierarchyViewModel::RebuildModelList() {
        if (!_modelApi) return;

        _state.filteredModels.clear();
        auto allModels = _modelApi->GetAllModels();

        std::string searchLower = _state.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto& mod : allModels) {
            std::string nameLower = mod.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (searchLower.empty() || nameLower.find(searchLower) != std::string::npos) {
                ModelNode node;
                node.id = mod.id;
                node.name = mod.name;
                node.path = mod.path;
                node.icon = SYN_ICON_CUBE;

                if (_previewApi) {
                    if (_previewApi->GetPreviewUVs(PreviewResourceType::Model, mod.id, node.uv0, node.uv1)) {
                        node.hasPreview = true;
                    }
                    else {
                        node.hasPreview = false;
                        _previewApi->RequestPreview(PreviewResourceType::Model, mod.id);
                    }
                }

                _state.filteredModels.push_back(node);
            }
        }
    }

    void ModelHierarchyViewModel::RebuildFlatList() {
        _state.flatNodes.clear();
        if (!_modelApi || _state.selectedModelId == 0xFFFFFFFF) return;

        if (const CpuModelData* cpuData = _modelApi->GetModelCpuData(_state.selectedModelId)) {

            std::unordered_map<uint16_t, std::vector<uint32_t>> childMap;
            for (uint32_t i = 0; i < cpuData->meshNodeDescriptors.size(); ++i) {
                childMap[cpuData->meshNodeDescriptors[i].parentNodeIndex].push_back(i);
            }

            std::string modelName = "Selected Model";
            auto allModels = _modelApi->GetAllModels();
            for (const auto& mod : allModels) {
                if (mod.id == _state.selectedModelId) {
                    modelName = mod.name;
                    break;
                }
            }

            TraverseAndFlatten(_state.selectedModelId, -1, 0xFFFF, 0, *cpuData, childMap, modelName);
        }
    }

    bool ModelHierarchyViewModel::TraverseAndFlatten(uint32_t modelId, int32_t descriptorIndex, uint16_t currentTransformIndex, int depth, const CpuModelData& cpuData, const std::unordered_map<uint16_t, std::vector<uint32_t>>& childMap, const std::string& nodeName) {

        bool matchesSearch = _state.searchQuery.empty() ||
            std::search(nodeName.begin(), nodeName.end(), _state.searchQuery.begin(), _state.searchQuery.end(),
                [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != nodeName.end();

        bool hasChildren = childMap.contains(currentTransformIndex);

        uint64_t key = GetNodeKey(modelId, descriptorIndex);
        bool isExpanded = _expandedNodes.contains(key) || !_state.searchQuery.empty();

        size_t flattenIndex = _state.flatNodes.size();

        ModelHierarchyNode node;
        node.modelId = modelId;
        node.descriptorIndex = descriptorIndex;
        node.name = nodeName;
        node.depth = depth;
        node.hasChildren = hasChildren;
        node.isExpanded = isExpanded;

        if (descriptorIndex == -1) {
            node.icon = SYN_ICON_CUBE;
            node.isMeshNode = false;
            node.triangleCount = cpuData.globalIndexCount / 3;
        }
        else {
            const auto& desc = cpuData.meshNodeDescriptors[descriptorIndex];
            node.icon = (desc.meshIndex != 0xFFFF) ? SYN_ICON_DRAW_POLYGON : SYN_ICON_PROJECT_DIAGRAM;
            node.isMeshNode = (desc.meshIndex != 0xFFFF);
            node.triangleCount = desc.indexCount / 3;
        }

        _state.flatNodes.push_back(node);

        bool anyChildMatches = false;

        if (isExpanded && hasChildren) {
            for (uint32_t childDescIdx : childMap.at(currentTransformIndex)) {
                const auto& childDesc = cpuData.meshNodeDescriptors[childDescIdx];
                std::string childName = childDesc.name;

                if (childName.empty())
                    childName = "Node_" + std::to_string(childDescIdx);

                if (TraverseAndFlatten(modelId, childDescIdx, childDesc.nodeIndex, depth + 1, cpuData, childMap, childName)) {
                    anyChildMatches = true;
                }
            }
        }

        if (!_state.searchQuery.empty() && !matchesSearch && !anyChildMatches) {
            _state.flatNodes.resize(flattenIndex);
            return false;
        }

        return true;
    }
}