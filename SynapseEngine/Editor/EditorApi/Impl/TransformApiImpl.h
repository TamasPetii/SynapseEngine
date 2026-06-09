#pragma once
#include "EditorCore/Api/ITransformApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class TransformApiImpl : public ITransformApi {
    public:
        TransformApiImpl(SceneManager* sm) : _sceneManager(sm) {}
        
        glm::vec3 GetEntityPosition(EntityID entity) const override;
        glm::vec3 GetEntityRotation(EntityID entity) const override;
        glm::vec3 GetEntityScale(EntityID entity) const override;

        void SetEntityPosition(EntityID entity, const glm::vec3& position) override;
        void SetEntityRotation(EntityID entity, const glm::vec3& rotation) override;
        void SetEntityScale(EntityID entity, const glm::vec3& scale) override;

        glm::mat4 GetEntityWorldMatrix(EntityID entity) const override;
    private:
        SceneManager* _sceneManager;
    };
}