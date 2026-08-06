#include "AnimationApiImpl.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h" 
#include "Editor/EditorApi/EditorApiUtils.h"
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

        for (EntityID entity : tagPool->GetDenseEntities()) {
            const auto& tag = tagPool->Get(entity);
            if (tag.tag == "Preview" && registry.HasComponent<AnimationComponent>(entity)) {
                EditorApiUtils::ModifyComponent<AnimationComponent>(
                    _sceneManager,
                    entity,
                    [animationId](auto& animationComp, auto pool) {
                        animationComp.animationIndex = animationId;
                    }
                );
            }
        }

        Syn::Info("AnimationApiImpl: Applied animation {} to preview object.", animationId);
    }
}