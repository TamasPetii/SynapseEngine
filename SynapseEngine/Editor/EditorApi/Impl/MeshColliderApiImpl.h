#pragma once
#include "EditorCore/Api/IMeshColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class MeshColliderApiImpl : public IMeshColliderApi {
    public:
        MeshColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasMeshCollider(EntityID entity) const override;

        uint32_t GetMeshColliderTargetLodLevel(EntityID entity) const override;
        glm::vec3 GetMeshColliderLocalOffset(EntityID entity) const override;

        void SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) override;
        void SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}