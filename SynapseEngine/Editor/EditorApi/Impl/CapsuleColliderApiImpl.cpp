#include "CapsuleColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"

namespace Syn {

    bool CapsuleColliderApiImpl::HasCapsuleCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<CapsuleColliderComponent>(_sceneManager, entity);
    }

    float CapsuleColliderApiImpl::GetCapsuleColliderRadius(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CapsuleColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.radius; }, 0.5f);
    }

    float CapsuleColliderApiImpl::GetCapsuleColliderHalfHeight(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CapsuleColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.halfHeight; }, 1.0f);
    }

    glm::vec3 CapsuleColliderApiImpl::GetCapsuleColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CapsuleColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void CapsuleColliderApiImpl::SetCapsuleColliderRadius(EntityID entity, float radius) {
        EditorApiUtils::ModifyComponent<CapsuleColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.radius = radius; });
    }

    void CapsuleColliderApiImpl::SetCapsuleColliderHalfHeight(EntityID entity, float halfHeight) {
        EditorApiUtils::ModifyComponent<CapsuleColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.halfHeight = halfHeight; });
    }

    void CapsuleColliderApiImpl::SetCapsuleColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<CapsuleColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}