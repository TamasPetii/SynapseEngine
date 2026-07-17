#include "AnimationApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Animation/AnimationManager.h"

namespace Syn {
    bool AnimationApiImpl::HasAnimation(EntityID entity) const {
        return EditorApiUtils::HasComponent<AnimationComponent>(_sceneManager, entity);
    }

    float AnimationApiImpl::GetAnimationSpeed(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AnimationComponent>(_sceneManager, entity, [](const auto& c) { return c.speed; }, 1.0f);
    }

    uint32_t AnimationApiImpl::GetAnimationIndex(EntityID entity) const {
        return EditorApiUtils::ReadComponent<AnimationComponent>(_sceneManager, entity, [](const auto& c) { return c.animationIndex; }, UINT32_MAX);
    }

    void AnimationApiImpl::SetAnimationSpeed(EntityID entity, float speed) {
        EditorApiUtils::ModifyComponent<AnimationComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.speed = speed; });
    }

    void AnimationApiImpl::SetAnimationIndex(EntityID entity, uint32_t index) {
        EditorApiUtils::ModifyComponent<AnimationComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            c.animationIndex = index;
            });
    }

    std::vector<std::pair<uint32_t, std::string>> AnimationApiImpl::GetAvailableAnimations() const {
        std::vector<std::pair<uint32_t, std::string>> result;

        auto animManager = ServiceLocator::Get<AnimationManager>();
        if (!animManager) {
            return result;
        }

        auto paths = animManager->GetResourcePaths();
        auto snapshots = animManager->GetResourceSnapshot();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (snapshots[i].state == ResourceState::Ready) {
                result.push_back({ i, paths[i] });
            }
        }

        return result;
    }
}