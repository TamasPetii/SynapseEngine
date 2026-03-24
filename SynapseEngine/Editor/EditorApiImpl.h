#pragma once
#include "EditorCore/API/IEditorAPI.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Component/TransformComponent.h"

namespace Syn {
    class EditorApiImpl : public IEditorAPI {
    public:
        EditorApiImpl(SceneManager* sceneManager) : _sceneManager(sceneManager) {}

        EntityID GetSelectedEntity() const override {
            return _selectedEntity;
        }

        void SetSelectedEntity(EntityID entity) override {
            _selectedEntity = entity;
        }

        // ITransformAPI

        glm::vec3 GetEntityPosition(EntityID entity) const override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            return transform.translation;
        }

        void SetEntityPosition(EntityID entity, const glm::vec3& position) override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            transform.translation = position;

            // TODO: Mátrix frissítési flag beállítása!
        }

        glm::vec3 GetEntityRotation(EntityID entity) const override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            return transform.rotation;
        }

        void SetEntityRotation(EntityID entity, const glm::vec3& rotation) override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            transform.rotation = rotation;

            // TODO: Mátrix frissítési flag beállítása!
        }

        glm::vec3 GetEntityScale(EntityID entity) const override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            return transform.scale;
        }

        void SetEntityScale(EntityID entity, const glm::vec3& scale) override {
            auto registry = _sceneManager->GetActiveScene()->GetRegistry();
            auto& transform = registry->GetComponent<TransformComponent>(entity);
            transform.scale = scale;

            // TODO: Mátrix frissítési flag beállítása!
        }

    private:
        SceneManager* _sceneManager;
        EntityID _selectedEntity = NULL_ENTITY;
    };
}