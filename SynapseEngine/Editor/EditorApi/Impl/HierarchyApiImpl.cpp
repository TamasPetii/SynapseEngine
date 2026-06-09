#include "HierarchyApiImpl.h"
#include "../EditorApiUtils.h"
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
    uint64_t HierarchyApiImpl::GetVersion() const {
        auto scene = _sceneManager->GetActiveScene();
        return (scene && scene->GetHierarchyManager()) ? scene->GetHierarchyManager()->GetVersion() : 0;
    }

    std::vector<EntityID> HierarchyApiImpl::GetRootEntities() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetHierarchyManager()) return {};
        auto rootSpan = scene->GetHierarchyManager()->GetEntitiesInLevel(0);
        return std::vector<EntityID>(rootSpan.begin(), rootSpan.end());
    }

    std::vector<EntityID> HierarchyApiImpl::GetChildren(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry() || !scene->GetRegistry()->HasComponent<HierarchyComponent>(entity)) return {};

        std::vector<EntityID> children;
        EntityID currChild = scene->GetRegistry()->GetComponent<HierarchyComponent>(entity).firstChild;
        while (currChild != NULL_ENTITY) {
            children.push_back(currChild);
            currChild = scene->GetRegistry()->GetComponent<HierarchyComponent>(currChild).nextSibling;
        }
        return children;
    }

    std::string HierarchyApiImpl::GetEntityIcon(EntityID entity) const {
        if (EditorApiUtils::HasComponent<CameraComponent>(_sceneManager, entity)) return SYN_ICON_VIDEO;
        if (EditorApiUtils::HasComponent<DirectionLightComponent>(_sceneManager, entity)) return ICON_FA_SUN;
        if (EditorApiUtils::HasComponent<PointLightComponent>(_sceneManager, entity)) return ICON_FA_LIGHTBULB;
        if (EditorApiUtils::HasComponent<SpotLightComponent>(_sceneManager, entity)) return ICON_FA_LIGHTBULB;
        if (EditorApiUtils::HasComponent<AnimationComponent>(_sceneManager, entity)) return ICON_FA_RUNNING;
        if (EditorApiUtils::HasComponent<ModelComponent>(_sceneManager, entity)) return SYN_ICON_CUBE;
        return SYN_ICON_CUBE;
    }

    bool HierarchyApiImpl::HasChildren(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry() || !scene->GetRegistry()->HasComponent<HierarchyComponent>(entity)) return false;
        return scene->GetRegistry()->GetComponent<HierarchyComponent>(entity).firstChild != NULL_ENTITY;
    }

    void HierarchyApiImpl::SetParent(EntityID child, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetHierarchyManager()) return;

        if (parent == NULL_ENTITY) scene->GetHierarchyManager()->DetachChild(child);
        else scene->GetHierarchyManager()->AttachChild(parent, child);
    }

    EntityID HierarchyApiImpl::CreateEntity(const std::string& name, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        EntityID newEntity = scene->CreateEntity();
        auto registry = scene->GetRegistry();

        registry->AddComponent<TagComponent>(newEntity);
        registry->GetComponent<TagComponent>(newEntity).name = name;
        registry->AddComponent<TransformComponent>(newEntity);

        if (parent != NULL_ENTITY) SetParent(newEntity, parent);

        return newEntity;
    }

    void HierarchyApiImpl::DestroyEntity(EntityID entity) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return;

        if (scene->GetSelectedEntity() == entity) {
            scene->SetSelectedEntity(NULL_ENTITY);
        }
        scene->DestroyEntity(entity);
    }

    EntityID HierarchyApiImpl::GetParent(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        auto registry = scene->GetRegistry();
        if (!registry->HasComponent<HierarchyComponent>(entity)) return NULL_ENTITY;

        return registry->GetComponent<HierarchyComponent>(entity).parent;
    }
}