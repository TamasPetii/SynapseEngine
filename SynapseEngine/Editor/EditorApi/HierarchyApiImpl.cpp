#include "EditorApiImpl.h"
#include "Editor/Manager/EditorIcons.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"

namespace Syn {

    std::vector<EntityID> EditorApiImpl::GetRootEntities() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return {};

        auto registry = scene->GetRegistry();

        const auto& denseEntities = registry->GetActiveEntities().GetDenseEntities();
        return std::vector<EntityID>(denseEntities.begin(), denseEntities.end());
    }

    std::vector<EntityID> EditorApiImpl::GetChildren(EntityID entity) const {
        return {};
    }

    std::string EditorApiImpl::GetEntityName(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (scene && scene->GetRegistry() && scene->GetRegistry()->HasComponent<TagComponent>(entity)) {
            return scene->GetRegistry()->GetComponent<TagComponent>(entity).name;
        }
        return "Entity " + std::to_string(entity);
    }

    std::string EditorApiImpl::GetEntityIcon(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return SYN_ICON_CUBE;

        auto registry = scene->GetRegistry();

        if (registry->HasComponent<CameraComponent>(entity)) return SYN_ICON_VIDEO;
        if (registry->HasComponent<DirectionLightComponent>(entity)) return ICON_FA_SUN;
        if (registry->HasComponent<PointLightComponent>(entity)) return ICON_FA_LIGHTBULB;
        if (registry->HasComponent<SpotLightComponent>(entity)) return ICON_FA_LIGHTBULB;
        if (registry->HasComponent<AnimationComponent>(entity)) return ICON_FA_RUNNING;
        if (registry->HasComponent<ModelComponent>(entity)) return SYN_ICON_CUBE;

        return SYN_ICON_CUBE;
    }

    bool EditorApiImpl::IsEntityVisible(EntityID entity) const {
        return !_hiddenEntities.contains(entity);
    }

    bool EditorApiImpl::HasChildren(EntityID entity) const {
        return false;
    }

    void EditorApiImpl::SetEntityVisibility(EntityID entity, bool visible) {
        if (visible) {
            _hiddenEntities.erase(entity);
        }
        else {
            _hiddenEntities.insert(entity);
        }

        //Todo
    }

    void EditorApiImpl::SetParent(EntityID child, EntityID parent) {
        // TODO: SetParent
    }

    EntityID EditorApiImpl::CreateEntity(const std::string& name, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        /*
        auto registry = scene->GetRegistry();
        EntityID newEntity = registry->CreateEntity();

        registry->AddComponents<TagComponent>(newEntity, { name });
        registry->AddComponents<TransformComponent>(newEntity);

        if (parent != NULL_ENTITY) {
            SetParent(newEntity, parent);
        }

        return newEntity;
        */

		return NULL_ENTITY;
    }

    void EditorApiImpl::DestroyEntity(EntityID entity) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return;

        if (_selectedEntity == entity) {
            _selectedEntity = NULL_ENTITY;
        }

        _hiddenEntities.erase(entity);
        scene->GetRegistry()->DestroyEntity(entity);
    }
}