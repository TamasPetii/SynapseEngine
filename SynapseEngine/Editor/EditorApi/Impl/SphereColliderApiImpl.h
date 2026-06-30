#pragma once
#include "EditorCore/Api/ISphereColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SphereColliderApiImpl : public ISphereColliderApi {
    public:
        SphereColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasSphereCollider(EntityID entity) const override;

        float GetSphereColliderRadius(EntityID entity) const override;
        glm::vec3 GetSphereColliderLocalOffset(EntityID entity) const override;

        void SetSphereColliderRadius(EntityID entity, float radius) override;
        void SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}