#include "ModelApiImpl.h"
#include <filesystem>

namespace Syn {

    std::vector<ModelItemData> ModelApiImpl::GetAllModels() const {
        if (!_modelManager) return {};

        std::vector<ModelItemData> result;
        auto paths = _modelManager->GetResourcePaths();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (_modelManager->GetEntryState(i) == ResourceState::Ready) {
                std::filesystem::path p(paths[i]);
                result.push_back({ i, p.filename().string(), paths[i] });
            }
        }
        return result;
    }

    uint64_t ModelApiImpl::GetVersion() const {
        return _modelManager ? _modelManager->GetVersion() : 0;
    }

    void ModelApiImpl::SetSelected(uint32_t modelId, int32_t nodeIndex) {
        _selectedModelId = modelId;
        _selectedNodeIndex = nodeIndex;
    }

    std::pair<uint32_t, int32_t> ModelApiImpl::GetSelected() const {
        return { _selectedModelId, _selectedNodeIndex };
    }

    const CpuModelData* ModelApiImpl::GetModelCpuData(uint32_t modelId) const {
        if (!_modelManager || modelId == INVALID_MODEL_ID) return nullptr;

        auto resource = _modelManager->GetResource(modelId);
        if (resource) {
            return &resource->cpuData;
        }

        return nullptr;
    }

    std::string ModelApiImpl::GetNodeName(uint32_t modelId, uint16_t nodeIndex) const {
        if (!_modelManager || modelId == INVALID_MODEL_ID) 
            return "Node_" + std::to_string(nodeIndex);

        auto resource = _modelManager->GetResource(modelId);
        return resource->cpuData.meshNodeDescriptors[nodeIndex].name;
    }
}