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
#include "Engine/Component/Components.h"
#include "Engine/Mesh/MeshSourceNames.h"

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
        if (EditorApiUtils::HasComponent<DirectionLightComponent>(_sceneManager, entity)) return SYN_ICON_SUN;
        if (EditorApiUtils::HasComponent<PointLightComponent>(_sceneManager, entity)) return SYN_ICON_LIGHTBULB;
        if (EditorApiUtils::HasComponent<SpotLightComponent>(_sceneManager, entity)) return SYN_ICON_SPOTLIGHT;
        if (EditorApiUtils::HasComponent<AnimationComponent>(_sceneManager, entity)) return SYN_ICON_RUNNING;
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

    EntityID HierarchyApiImpl::CreateEntity(EntityTemplate templateType, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        EntityID newEntity = scene->CreateEntity();
        auto registry = scene->GetRegistry();

        registry->AddComponent<TagComponent>(newEntity);
        registry->AddComponent<TransformComponent>(newEntity);

        registry->GetPool<TagComponent>()->SetCategory(newEntity, StorageCategory::Static);
        registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Static);

        std::string name = "Entity";

        switch (templateType) {
        case EntityTemplate::Empty:
            name = "Empty Entity";
            break;

        case EntityTemplate::Camera:
            name = "Camera";
            registry->AddComponent<CameraComponent>(newEntity);
            registry->GetPool<CameraComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            break;

        case EntityTemplate::DirectionalLight:
            name = "Directional Light";
            registry->AddComponent<DirectionLightComponent>(newEntity);
            registry->GetPool<DirectionLightComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<DirectionLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(newEntity);
            registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(newEntity);
            break;
        case EntityTemplate::PointLight:
            name = "Point Light";
            registry->AddComponent<PointLightComponent>(newEntity);
            registry->GetPool<PointLightComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<PointLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(newEntity);
            registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(newEntity);
            break;
        case EntityTemplate::SpotLight:
            name = "Spot Light";
            registry->AddComponent<SpotLightComponent>(newEntity);
            registry->GetPool<SpotLightComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<SpotLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(newEntity);
            registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(newEntity);
            break;

        case EntityTemplate::BoxCollider:
            name = "Box Collider";
            registry->AddComponent<BoxColliderComponent>(newEntity);
            registry->AddComponent<RigidBodyComponent>(newEntity);
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Cube);

            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<RigidBodyComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<BoxColliderComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::SphereCollider:
            name = "Sphere Collider";
            registry->AddComponent<SphereColliderComponent>(newEntity);
            registry->AddComponent<RigidBodyComponent>(newEntity);
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Sphere);

            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<RigidBodyComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<SphereColliderComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::CapsuleCollider:
            name = "Capsule Collider";
            registry->AddComponent<CapsuleColliderComponent>(newEntity);
            registry->AddComponent<RigidBodyComponent>(newEntity);
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Capsule);

            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<RigidBodyComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<CapsuleColliderComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ConvexCollider:
            name = "Convex Collider";
            registry->AddComponent<ConvexColliderComponent>(newEntity);
            registry->AddComponent<RigidBodyComponent>(newEntity);
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);

            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<RigidBodyComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ConvexColliderComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::MeshCollider:
            name = "Mesh Collider";
            registry->AddComponent<MeshColliderComponent>(newEntity);
            registry->AddComponent<RigidBodyComponent>(newEntity);
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);

            registry->GetPool<TransformComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<RigidBodyComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MeshColliderComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;

        case EntityTemplate::ShapeCube:
            name = "Cube";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Cube);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeSphere:
            name = "Sphere";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Sphere);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeQuad:
            name = "Quad";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Quad);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeCylinder:
            name = "Cylinder";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Cylinder);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeCone:
            name = "Cone";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Cone);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeCapsule:
            name = "Capsule";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Capsule);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeHemisphere:
            name = "Hemisphere";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Hemisphere);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapePyramid:
            name = "Pyramid";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Pyramid);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeGrid:
            name = "Grid";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Grid);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::ShapeTorus:
            name = "Torus";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetComponent<ModelComponent>(newEntity).modelIndex = _modelManager->GetResourceIndex(MeshSourceNames::Torus);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;

        case EntityTemplate::Model:
            name = "Model";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        case EntityTemplate::Animation:
            name = "Animation";
            registry->AddComponent<MaterialOverrideComponent>(newEntity);
            registry->AddComponent<AnimationComponent>(newEntity);
            registry->AddComponent<ModelComponent>(newEntity);

            registry->GetPool<AnimationComponent>()->SetCategory(newEntity, StorageCategory::Stream);
            registry->GetPool<ModelComponent>()->SetCategory(newEntity, StorageCategory::Static);
            registry->GetPool<MaterialOverrideComponent>()->SetCategory(newEntity, StorageCategory::Static);
            break;
        }

        registry->GetComponent<TagComponent>(newEntity).name = name;

        if (parent != NULL_ENTITY) {
            SetParent(newEntity, parent);
        }

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

    void HierarchyApiImpl::DestroyEntityRecursive(EntityID entity) {
        auto children = GetChildren(entity);
        for (EntityID child : children) {
            DestroyEntityRecursive(child);
        }
        DestroyEntity(entity);
    }

    void HierarchyApiImpl::DestroyEntityKeepChildren(EntityID entity) {
        EntityID parent = GetParent(entity);
        auto children = GetChildren(entity);

        for (EntityID child : children) {
            SetParent(child, parent);
        }

        DestroyEntity(entity);
    }

    EntityID HierarchyApiImpl::CopyEntity(EntityID entity, EntityID parent) {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        EntityID newEntity = scene->CreateEntity();
        auto registry = scene->GetRegistry();

        EditorApiUtils::CloneEntityComponents<
            TagComponent,
            TransformComponent,
            CameraComponent,
            ModelComponent,
            AnimationComponent,
            MaterialOverrideComponent,
            DirectionLightComponent,
            PointLightComponent,
            SpotLightComponent,
            DirectionLightShadowComponent,
            PointLightShadowComponent,
            SpotLightShadowComponent,
            BoxColliderComponent,
            SphereColliderComponent,
            CapsuleColliderComponent,
            ConvexColliderComponent,
            MeshColliderComponent,
            RigidBodyComponent
        >(_sceneManager, entity, newEntity);

        if (registry->HasComponent<TagComponent>(newEntity)) {
            registry->GetComponent<TagComponent>(newEntity).name += " (Copy)";
        }

        if (registry->HasComponent<TransformComponent>(newEntity)) {
            registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(newEntity);
        }

        if (parent != NULL_ENTITY) {
            SetParent(newEntity, parent);
        }

        return newEntity;
    }

    EntityID HierarchyApiImpl::FullCopyEntity(EntityID entity, EntityID parent) {
        EntityID newEntity = CopyEntity(entity, parent);

        auto children = GetChildren(entity);
        for (EntityID child : children) {
            FullCopyEntity(child, newEntity);
        }

        return newEntity;
    }

    EntityID HierarchyApiImpl::GetParent(EntityID entity) const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return NULL_ENTITY;

        auto registry = scene->GetRegistry();
        if (!registry->HasComponent<HierarchyComponent>(entity)) return NULL_ENTITY;

        return registry->GetComponent<HierarchyComponent>(entity).parent;
    }
}