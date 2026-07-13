#pragma once
#include "EditorCore/Api/ICapsuleColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class CapsuleColliderApiImpl : public ICapsuleColliderApi {
    public:
        CapsuleColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasCapsuleCollider(EntityID entity) const override;

        float GetCapsuleColliderRadius(EntityID entity) const override;
        float GetCapsuleColliderHalfHeight(EntityID entity) const override;
        glm::vec3 GetCapsuleColliderLocalOffset(EntityID entity) const override;

        void SetCapsuleColliderRadius(EntityID entity, float radius) override;
        void SetCapsuleColliderHalfHeight(EntityID entity, float halfHeight) override;
        void SetCapsuleColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}