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

#include "AnimationApiImpl.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h" 
#include "Editor/EditorApi/EditorApiUtils.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Logger/SynLog.h"
#include <filesystem>
#include <algorithm>

namespace Syn {

    std::vector<AnimationItemData> AnimationApiImpl::GetAllAnimations() const {
        if (!_animManager) return {};

        std::vector<AnimationItemData> result;
        auto paths = _animManager->GetResourcePaths();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (_animManager->GetEntryState(i) == ResourceState::Ready) {
                std::filesystem::path p(paths[i]);
                result.push_back({ i, p.filename().string(), paths[i] });
            }
        }
        return result;
    }

    uint64_t AnimationApiImpl::GetVersion() const {
        return _animManager ? _animManager->GetVersion() : 0;
    }

    void AnimationApiImpl::SetSelected(uint32_t animationId) {
        _selectedAnimationId = animationId;
    }

    uint32_t AnimationApiImpl::GetSelected() const {
        return _selectedAnimationId;
    }

    const CpuAnimationData* AnimationApiImpl::GetAnimationCpuData(uint32_t animationId) const {
        if (!_animManager || animationId == INVALID_ANIMATION_ID) return nullptr;

        auto resource = _animManager->GetResource(animationId);
        if (resource) {
            return &resource->cpuData;
        }

        return nullptr;
    }

    void AnimationApiImpl::ApplyAnimationToPreviewObject(uint32_t animationId) {
        if (!_sceneManager) return;
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return;

        auto& registry = SceneInsider::GetRegistry(*scene, SceneInsider::GetKey());
        auto tagPool = registry.GetPool<TagComponent>();
        if (!tagPool) return;

        uint32_t baseModelId = 0xFFFFFFFF;

        if (animationId != INVALID_ANIMATION_ID && _animManager) {
            auto animResource = _animManager->GetResource(animationId);
            if (animResource) {
                baseModelId = animResource->cpuData.baseModelId;
            }
        }

        for (EntityID entity : tagPool->GetDenseEntities()) {
            const auto& tag = tagPool->Get(entity);
            if (tag.tag == "Preview") {

                if (registry.HasComponent<AnimationComponent>(entity)) {
                    EditorApiUtils::ModifyComponent<AnimationComponent>(
                        _sceneManager,
                        entity,
                        [animationId](auto& animatorComp, auto pool) {
                            animatorComp.animationIndex = animationId;
                            animatorComp.time = 0.0f;
                        }
                    );
                }

                if (registry.HasComponent<ModelComponent>(entity)) {
                    EditorApiUtils::ModifyComponent<ModelComponent>(
                        _sceneManager,
                        entity,
                        [baseModelId](auto& modelComp, auto pool) {
                            modelComp.modelIndex = baseModelId;
                        }
                    );
                }
            }
        }

        if (baseModelId != 0xFFFFFFFF) {
            auto modelManager = ServiceLocator::Get<ModelManager>();
            if (modelManager) {
                auto modelRes = modelManager->GetResource(baseModelId);
                if (modelRes) {
                    glm::vec3 center = modelRes->cpuData.globalCollider.center;
                    float radius = modelRes->cpuData.globalCollider.radius * 1.05f;

                    float targetSize = radius;
                    if (targetSize > 100) targetSize = 100.0f;
                    float scaleFactor = targetSize / (radius * 2.0f);

                    for (EntityID entity : tagPool->GetDenseEntities()) {
                        const auto& tag = tagPool->Get(entity);

                        if (tag.tag == "Preview" && registry.HasComponent<TransformComponent>(entity)) {
                            EditorApiUtils::ModifyComponent<TransformComponent>(
                                _sceneManager,
                                entity,
                                [scaleFactor](auto& transformComp, auto pool) {
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
        }

        Syn::Info("AnimationApiImpl: Applied animation {} and base model {} to preview object.", animationId, baseModelId);
    }
}