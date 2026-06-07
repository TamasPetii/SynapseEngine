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

namespace Syn 
{
    uint64_t EditorApiImpl::GetVersion() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetHierarchyManager()) return 0;
        return scene->GetHierarchyManager()->GetVersion();
    }

    std::vector<EntityID> EditorApiImpl::GetRootEntities() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetHierarchyManager()) return {};

        auto rootSpan = scene->GetHierarchyManager()->GetEntitiesInLevel(0);
        return std::vector<EntityID>(rootSpan.begin(), rootSpan.end());
    }

    std::vector<EntityID> EditorApiImpl::GetChildren(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return {};

        auto registry = scene->GetRegistry();
        if (!registry->HasComponent<HierarchyComponent>(entity)) return {};

        std::vector<EntityID> children;
        EntityID currChild = registry->GetComponent<HierarchyComponent>(entity).firstChild;

        while (currChild != NULL_ENTITY) {
            children.push_back(currChild);
            currChild = registry->GetComponent<HierarchyComponent>(currChild).nextSibling;
        }

        return children;
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

    bool EditorApiImpl::HasChildren(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return false;

        auto registry = scene->GetRegistry();
        if (!registry->HasComponent<HierarchyComponent>(entity)) return false;

        return registry->GetComponent<HierarchyComponent>(entity).firstChild != NULL_ENTITY;
    }

    void EditorApiImpl::SetParent(EntityID child, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetHierarchyManager()) return;

        if (parent == NULL_ENTITY) {
            scene->GetHierarchyManager()->DetachChild(child);
        }
        else {
            scene->GetHierarchyManager()->AttachChild(parent, child);
        }
    }

    EntityID EditorApiImpl::CreateEntity(const std::string& name, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        auto registry = scene->GetRegistry();
        EntityID newEntity = scene->CreateEntity();

        registry->AddComponent<TagComponent>(newEntity);
        registry->GetComponent<TagComponent>(newEntity).name = name;
        registry->AddComponent<TransformComponent>(newEntity);

        if (parent != NULL_ENTITY) {
            SetParent(newEntity, parent);
        }

        return newEntity;
    }

    void EditorApiImpl::DestroyEntity(EntityID entity) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return;

        if (_selectedEntity == entity) {
            _selectedEntity = NULL_ENTITY;
        }

        scene->DestroyEntity(entity);
    }
}