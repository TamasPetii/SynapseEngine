#include "EditorApiImpl.h"

namespace Syn {
    glm::vec3 EditorApiImpl::GetEntityPosition(EntityID entity) const {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.translation;
    }

    void EditorApiImpl::SetEntityPosition(EntityID entity, const glm::vec3& position) {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.translation = position;

        // TODO: Mátrix frissítési flag beállítása!
    }

    glm::vec3 EditorApiImpl::GetEntityRotation(EntityID entity) const {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.rotation;
    }

    void EditorApiImpl::SetEntityRotation(EntityID entity, const glm::vec3& rotation) {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.rotation = rotation;

        // TODO: Mátrix frissítési flag beállítása!
    }

    glm::vec3 EditorApiImpl::GetEntityScale(EntityID entity) const {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        return transform.scale;
    }

    void EditorApiImpl::SetEntityScale(EntityID entity, const glm::vec3& scale) {
        auto registry = _sceneManager->GetActiveScene()->GetRegistry();
        auto& transform = registry->GetComponent<TransformComponent>(entity);
        transform.scale = scale;

        // TODO: Mátrix frissítési flag beállítása!
    }
};