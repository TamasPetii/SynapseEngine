#include "BoxColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"

namespace Syn {

    bool BoxColliderApiImpl::HasBoxCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<BoxColliderComponent>(_sceneManager, entity);
    }

    glm::vec3 BoxColliderApiImpl::GetBoxColliderHalfExtents(EntityID entity) const {
        return EditorApiUtils::ReadComponent<BoxColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.halfExtents; }, glm::vec3(0.5f));
    }

    glm::vec3 BoxColliderApiImpl::GetBoxColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<BoxColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void BoxColliderApiImpl::SetBoxColliderHalfExtents(EntityID entity, const glm::vec3& halfExtents) {
        EditorApiUtils::ModifyComponent<BoxColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.halfExtents = halfExtents; });
    }

    void BoxColliderApiImpl::SetBoxColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<BoxColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}