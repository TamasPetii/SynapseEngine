#pragma once
#include "EditorCore/Api/IEditorAPI.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Component/TransformComponent.h"

namespace Syn {
    class EditorApiImpl : public IEditorAPI {
    public:
        EditorApiImpl(SceneManager* sceneManager) : _sceneManager(sceneManager) {}

        //ISelectAPI
        EntityID GetSelectedEntity() const override;
        void SetSelectedEntity(EntityID entity) override;

        // ITransformAPI
        glm::vec3 GetEntityScale(EntityID entity) const override;
        glm::vec3 GetEntityPosition(EntityID entity) const override;
        glm::vec3 GetEntityRotation(EntityID entity) const override;
        void SetEntityScale(EntityID entity, const glm::vec3& scale) override;
        void SetEntityRotation(EntityID entity, const glm::vec3& rotation) override;
        void SetEntityPosition(EntityID entity, const glm::vec3& position) override;

    private:
        SceneManager* _sceneManager;
        EntityID _selectedEntity = NULL_ENTITY;
    };
}