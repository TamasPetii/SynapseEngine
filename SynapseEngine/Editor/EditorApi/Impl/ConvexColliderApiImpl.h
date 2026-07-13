#pragma once
#include "EditorCore/Api/IConvexColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class ConvexColliderApiImpl : public IConvexColliderApi {
    public:
        ConvexColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasConvexCollider(EntityID entity) const override;

        uint32_t GetConvexColliderTargetLodLevel(EntityID entity) const override;
        glm::vec3 GetConvexColliderLocalOffset(EntityID entity) const override;

        void SetConvexColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) override;
        void SetConvexColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}