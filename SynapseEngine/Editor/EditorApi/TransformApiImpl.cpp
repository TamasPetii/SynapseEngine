#include "EditorApiImpl.h"
#include "Engine/Component/TransformComponent.h"

namespace Syn {
    glm::vec3 EditorApiImpl::GetEntityPosition(EntityID entity) const {
        constexpr auto nullValue = glm::vec3(0.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return nullValue;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return nullValue;
        
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.translation;
    }

    void EditorApiImpl::SetEntityPosition(EntityID entity, const glm::vec3& position) {
        constexpr auto nullValue = glm::vec3(0.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return;

        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.translation = position;

        // TODO: Mátrix frissítési flag beállítása!
    }

    glm::vec3 EditorApiImpl::GetEntityRotation(EntityID entity) const {
        constexpr auto nullValue = glm::vec3(0.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return nullValue;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return nullValue;

        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.rotation;
    }

    void EditorApiImpl::SetEntityRotation(EntityID entity, const glm::vec3& rotation) {
        constexpr auto nullValue = glm::vec3(0.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return;

        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.rotation = rotation;

        // TODO: Mátrix frissítési flag beállítása!
    }

    glm::vec3 EditorApiImpl::GetEntityScale(EntityID entity) const {
        constexpr auto nullValue = glm::vec3(1.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return nullValue;

        auto registry = scene->GetRegistry();

        if (!registry->HasComponent<TransformComponent>(entity))
            return nullValue;

        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.scale;
    }

    void EditorApiImpl::SetEntityScale(EntityID entity, const glm::vec3& scale) {
        constexpr auto nullValue = glm::vec3(0.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return;

        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.scale = scale;

        // TODO: Mátrix frissítési flag beállítása!
    }

    glm::mat4 EditorApiImpl::GetEntityWorldMatrix(EntityID entity) const {
        constexpr auto nullValue = glm::mat4(1.0f);

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return nullValue;

        auto registry = scene->GetRegistry();

        if (registry == nullptr || !registry->HasComponent<TransformComponent>(entity))
            return nullValue;

        return registry->GetComponent<TransformComponent>(entity).transform;
    }

    EntityID EditorApiImpl::GetEntityParent(EntityID entity) const {
        constexpr auto nullValue = NULL_ENTITY;

        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return nullValue;

        auto registry = scene->GetRegistry();

        if (registry == nullptr)
            return nullValue;

        /* 
        if (registry->HasComponent<Relationship>(entity)) {
            return registry->GetComponent<Relationship>(entity).parent;
        }
        */
    }
};