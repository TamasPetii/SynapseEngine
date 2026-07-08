#include "ModelApiImpl.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Editor/EditorApi/EditorApiUtils.h"
#include "Engine/Logger/SynLog.h"
#include <filesystem>
#include <algorithm>

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

    void ModelApiImpl::ApplyModelToPreviewObject(uint32_t modelId) {
        if (!_sceneManager) return;
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return;

        auto& registry = SceneInsider::GetRegistry(*scene, SceneInsider::GetKey());
        auto tagPool = registry.GetPool<TagComponent>();
        if (!tagPool) return;

        for (EntityID entity : tagPool->GetDenseEntities()) {
            const auto& tag = tagPool->Get(entity);
            if (tag.tag == "Preview" && registry.HasComponent<ModelComponent>(entity)) {
                EditorApiUtils::ModifyComponent<ModelComponent>(
                    _sceneManager,
                    entity,
                    [modelId](auto& modelComp, auto pool) {
                        modelComp.modelIndex = modelId;
                    }
                );
            }
        }

        if (modelId != INVALID_MODEL_ID && _modelManager) {
            auto resource = _modelManager->GetResource(modelId);
            if (resource) {
                glm::vec3 center = resource->cpuData.globalCollider.center;
                float radius = resource->cpuData.globalCollider.radius;

                for (EntityID entity : tagPool->GetDenseEntities()) {
                    const auto& tag = tagPool->Get(entity);

                    if (tag.tag == "Camera" && registry.HasComponent<CameraComponent>(entity)) {

                        EditorApiUtils::ModifyComponent<CameraComponent>(
                            _sceneManager,
                            entity,
                            [center, radius](auto& camComp, auto pool) {
                                camComp.target = center;
                                camComp.distance = std::max(radius * 2.5f, 2.0f);
                            }
                        );
                        break;
                    }
                }
            }
        }

        Syn::Info("ModelApiImpl: Applied model {} to preview object and adjusted camera.", modelId);
    }
}