#pragma once
#include "EditorCore/Api/IBoxColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class BoxColliderApiImpl : public IBoxColliderApi {
    public:
        BoxColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasBoxCollider(EntityID entity) const override;

        glm::vec3 GetBoxColliderHalfExtents(EntityID entity) const override;
        glm::vec3 GetBoxColliderLocalOffset(EntityID entity) const override;

        void SetBoxColliderHalfExtents(EntityID entity, const glm::vec3& halfExtents) override;
        void SetBoxColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}