#include "MeshColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"

namespace Syn {

    bool MeshColliderApiImpl::HasMeshCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<MeshColliderComponent>(_sceneManager, entity);
    }

    uint32_t MeshColliderApiImpl::GetMeshColliderTargetLodLevel(EntityID entity) const {
        return EditorApiUtils::ReadComponent<MeshColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.targetLodLevel; }, 0u);
    }

    glm::vec3 MeshColliderApiImpl::GetMeshColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<MeshColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void MeshColliderApiImpl::SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) {
        EditorApiUtils::ModifyComponent<MeshColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.targetLodLevel = targetLodLevel; });
    }

    void MeshColliderApiImpl::SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<MeshColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}