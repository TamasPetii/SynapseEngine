#include "ModelHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h"
#include <algorithm>
#include <cctype>

namespace Syn {
    ModelHierarchyViewModel::ModelHierarchyViewModel(IModelApi* modelApi)
        : _modelApi(modelApi)
    {}

    uint64_t ModelHierarchyViewModel::GetNodeKey(uint32_t modelId, int32_t descriptorIndex) const {
        return (static_cast<uint64_t>(modelId) << 32) | static_cast<uint32_t>(descriptorIndex);
    }

    void ModelHierarchyViewModel::SyncWithEngine() {
        if (!_modelApi) return;

        uint64_t currentVersion = _modelApi->GetVersion();
        auto selection = _modelApi->GetSelected();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildFlatList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_state.selectedModelId != selection.first || _state.selectedDescriptorIndex != selection.second) {
            _state.selectedModelId = selection.first;
            _state.selectedDescriptorIndex = selection.second;
        }
    }

    void ModelHierarchyViewModel::Dispatch(const ModelHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, ModelHierarchySelectIntent>) {
                if (_modelApi)
                {
                    _modelApi->SetSelected(arg.modelId, arg.descriptorIndex);
                    _modelApi->ApplyModelToPreviewObject(arg.modelId);
                }

                _state.selectedModelId = arg.modelId;
                _state.selectedDescriptorIndex = arg.descriptorIndex;
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

    void ModelHierarchyViewModel::RebuildFlatList() {
        if (!_modelApi) return;

        _state.flatNodes.clear();
        auto allModels = _modelApi->GetAllModels();

        for (const auto& mod : allModels) {
            if (const CpuModelData* cpuData = _modelApi->GetModelCpuData(mod.id)) {

                std::unordered_map<uint16_t, std::vector<uint32_t>> childMap;
                for (uint32_t i = 0; i < cpuData->meshNodeDescriptors.size(); ++i) {
                    childMap[cpuData->meshNodeDescriptors[i].parentNodeIndex].push_back(i);
                }

                TraverseAndFlatten(mod.id, -1, 0xFFFF, 0, *cpuData, childMap, mod.name);
            }
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