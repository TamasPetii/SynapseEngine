#include "EditorApiImpl.h"
#include "Engine/Component/Core/TagComponent.h" 

namespace Syn {

    std::string EditorApiImpl::GetEntityTag(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return "Unknown";

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return "Unknown";

        if (registry->HasComponent<TagComponent>(entity)) {
            return registry->GetComponent<TagComponent>(entity).tag;
        }

        return "Untagged";
    }

    void EditorApiImpl::SetEntityTag(EntityID entity, const std::string& tag) {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return;

        if (registry->HasComponent<TagComponent>(entity)) {
            registry->GetComponent<TagComponent>(entity).tag = tag;
        }
    }

    std::string EditorApiImpl::GetEntityName(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return "Unknown";

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return "Unknown";

        if (registry->HasComponent<TagComponent>(entity)) {
            return registry->GetComponent<TagComponent>(entity).name;
        }

        return "Entity " + std::to_string(entity);
    }

    void EditorApiImpl::SetEntityName(EntityID entity, const std::string& name) {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return;

        if (registry->HasComponent<TagComponent>(entity)) {
            registry->GetComponent<TagComponent>(entity).name = name;
        }
    }

    bool EditorApiImpl::IsEntityEnabled(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return false;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return false;

        if (registry->HasComponent<TagComponent>(entity)) {
            return registry->GetComponent<TagComponent>(entity).enabled;
        }
        return true;
    }

    void EditorApiImpl::SetEntityEnabled(EntityID entity, bool enabled) {
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return;

        if (registry->HasComponent<TagComponent>(entity)) {
            registry->GetComponent<TagComponent>(entity).enabled = enabled;
        }
    }
}