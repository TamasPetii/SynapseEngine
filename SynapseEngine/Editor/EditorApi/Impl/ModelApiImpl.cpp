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

#include "ModelApiImpl.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
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
                float radius = resource->cpuData.globalCollider.radius * 1.05f;

                float targetSize = radius;

                if (targetSize > 100)
                    targetSize = 100.0f;

                float scaleFactor = targetSize / (radius * 2.0f);

                for (EntityID entity : tagPool->GetDenseEntities()) {
                    const auto& tag = tagPool->Get(entity);

                    if (tag.tag == "Preview" && registry.HasComponent<TransformComponent>(entity)) {
                        EditorApiUtils::ModifyComponent<TransformComponent>(
                            _sceneManager,
                            entity,
                            [center, scaleFactor](auto& transformComp, auto pool) {
                                transformComp.scale = glm::vec3(scaleFactor);
                            }
                        );
                    }

                    if (tag.tag == "Camera" && registry.HasComponent<CameraComponent>(entity)) {
                        EditorApiUtils::ModifyComponent<CameraComponent>(
                            _sceneManager,
                            entity,
                            [targetSize](auto& camComp, auto pool) {
                                camComp.target = glm::vec3(0.0f);
                                camComp.distance = targetSize * 1.25f;
                            }
                        );
                    }
                }
            }
        }

        Syn::Info("ModelApiImpl: Applied model {} to preview object and adjusted camera.", modelId);
    }
}