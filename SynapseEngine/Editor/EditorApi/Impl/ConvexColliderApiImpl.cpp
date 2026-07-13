#include "ConvexColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/ConvexColliderComponent.h"

namespace Syn {

    bool ConvexColliderApiImpl::HasConvexCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<ConvexColliderComponent>(_sceneManager, entity);
    }

    uint32_t ConvexColliderApiImpl::GetConvexColliderTargetLodLevel(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ConvexColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.targetLodLevel; }, 0u);
    }

    glm::vec3 ConvexColliderApiImpl::GetConvexColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ConvexColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void ConvexColliderApiImpl::SetConvexColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) {
        EditorApiUtils::ModifyComponent<ConvexColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.targetLodLevel = targetLodLevel; });
    }

    void ConvexColliderApiImpl::SetConvexColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<ConvexColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}