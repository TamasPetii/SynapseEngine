#include "SphereColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"

namespace Syn {

    bool SphereColliderApiImpl::HasSphereCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<SphereColliderComponent>(_sceneManager, entity);
    }

    float SphereColliderApiImpl::GetSphereColliderRadius(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SphereColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.radius; }, 0.5f);
    }

    glm::vec3 SphereColliderApiImpl::GetSphereColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SphereColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void SphereColliderApiImpl::SetSphereColliderRadius(EntityID entity, float radius) {
        EditorApiUtils::ModifyComponent<SphereColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.radius = radius; });
    }

    void SphereColliderApiImpl::SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<SphereColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}