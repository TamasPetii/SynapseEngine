#include "AnimationPreviewSceneSource.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Utils/PathUtils.h"

#include <glm/gtc/constants.hpp>
#include <limits>

namespace Syn
{
    bool AnimationPreviewSceneSource::Populate(Scene& scene)
    {
        Registry& registry = SceneInsider::GetRegistry(scene, SceneInsider::GetKey());
        EntityID& sceneCam = SceneInsider::GetSceneCameraEntity(scene, SceneInsider::GetKey());
        HierarchyManager* hm = scene.GetHierarchyManager();

        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto materialManager = ServiceLocator::Get<MaterialManager>();

        Syn::Info("Populating Animation Preview Scene...");

        // Root entities
        EntityID rootCameras = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootCameras);
        registry.GetComponent<TagComponent>(rootCameras).name = "Cameras";
        registry.GetComponent<TagComponent>(rootCameras).tag = "Root";
        registry.AddComponent<TransformComponent>(rootCameras);
        registry.GetPool<TransformComponent>()->SetCategory(rootCameras, StorageCategory::Static);

        EntityID rootEnvironment = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootEnvironment);
        registry.GetComponent<TagComponent>(rootEnvironment).name = "Preview Objects";
        registry.GetComponent<TagComponent>(rootEnvironment).tag = "Root";
        registry.AddComponent<TransformComponent>(rootEnvironment);
        registry.GetPool<TransformComponent>()->SetCategory(rootEnvironment, StorageCategory::Static);

        EntityID rootLights = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootLights);
        registry.GetComponent<TagComponent>(rootLights).name = "Studio Lights";
        registry.GetComponent<TagComponent>(rootLights).tag = "Root";
        registry.AddComponent<TransformComponent>(rootLights);
        registry.GetPool<TransformComponent>()->SetCategory(rootLights, StorageCategory::Static);

        // Orbit camera
        sceneCam = scene.CreateEntity();
        registry.AddComponent<TagComponent>(sceneCam);
        registry.GetComponent<TagComponent>(sceneCam).name = "Preview Camera";
        registry.GetComponent<TagComponent>(sceneCam).tag = "Camera";

        registry.AddComponent<TransformComponent>(sceneCam);
        auto& camTransform = registry.GetComponent<TransformComponent>(sceneCam);
        camTransform.rotation = glm::vec3(-25.0f, 45.0f, 0.0f);

        registry.AddComponent<CameraComponent>(sceneCam);
        auto& camComp = registry.GetComponent<CameraComponent>(sceneCam);
        camComp.useOrbit = true;
        camComp.target = glm::vec3(0.0f, 0.5f, 0.0f);
        camComp.distance = 12.0f;
        camComp.speed = 20.0f;

        registry.GetPool<CameraComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
        registry.GetPool<TransformComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
        hm->AttachChild(rootCameras, sceneCam);

        // 1. Key light
        EntityID keyLight = scene.CreateEntity();
        registry.AddComponent<TagComponent>(keyLight);
        registry.GetComponent<TagComponent>(keyLight).name = "Key Light";

        registry.AddComponent<TransformComponent>(keyLight);
        registry.GetComponent<TransformComponent>(keyLight).rotation = glm::vec3(-45.0f, -45.0f, 0.0f);

        registry.AddComponent<DirectionLightComponent>(keyLight);
        auto& dirKey = registry.GetComponent<DirectionLightComponent>(keyLight);
        dirKey.color = glm::vec3(1.0f, 0.95f, 0.9f);
        dirKey.strength = 3.0f;
        dirKey.useShadow = true;

        registry.GetPool<DirectionLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(keyLight);
        hm->AttachChild(rootLights, keyLight);

        // 2. Fill light
        EntityID fillLight = scene.CreateEntity();
        registry.AddComponent<TagComponent>(fillLight);
        registry.GetComponent<TagComponent>(fillLight).name = "Fill Light";

        registry.AddComponent<TransformComponent>(fillLight);
        registry.GetComponent<TransformComponent>(fillLight).rotation = glm::vec3(30.0f, 135.0f, 0.0f);

        registry.AddComponent<DirectionLightComponent>(fillLight);
        auto& dirFill = registry.GetComponent<DirectionLightComponent>(fillLight);
        dirFill.color = glm::vec3(0.8f, 0.9f, 1.0f);
        dirFill.strength = 1.0f;
        dirFill.useShadow = false;

        hm->AttachChild(rootLights, fillLight);

        // Preview Animation Entity
        EntityID previewAnimation = scene.CreateEntity();
        registry.AddComponent<TagComponent>(previewAnimation);
        registry.GetComponent<TagComponent>(previewAnimation).name = "Preview_Animation";
        registry.GetComponent<TagComponent>(previewAnimation).tag = "Preview";
        registry.GetPool<TagComponent>()->SetCategory(previewAnimation, StorageCategory::Static);

        registry.AddComponent<TransformComponent>(previewAnimation);
        registry.GetComponent<TransformComponent>(previewAnimation).translation = glm::vec3(0.0f, 0.0f, 0.0f);
        registry.GetComponent<TransformComponent>(previewAnimation).scale = glm::vec3(1.0f);
        registry.GetPool<TransformComponent>()->SetCategory(previewAnimation, StorageCategory::Static);

        registry.AddComponent<ModelComponent>(previewAnimation);
        registry.GetComponent<ModelComponent>(previewAnimation).modelIndex = 0xFFFFFFFF;
        registry.GetPool<ModelComponent>()->SetCategory(previewAnimation, StorageCategory::Static);

        registry.AddComponent<AnimationComponent>(previewAnimation);
        registry.GetComponent<AnimationComponent>(previewAnimation).animationIndex = 0xFFFFFFFF;
        registry.GetPool<AnimationComponent>()->SetCategory(previewAnimation, StorageCategory::Stream); 

        registry.AddComponent<MaterialOverrideComponent>(previewAnimation);
        registry.GetPool<MaterialOverrideComponent>()->SetCategory(previewAnimation, StorageCategory::Static);

        hm->AttachChild(rootEnvironment, previewAnimation);

        auto skyTextureId = ServiceLocator::Get<ImageManager>()->LoadImageSync(PathUtils::GetAbsolutePathString("Assets/Engine/Environment/ModelPreview.hdr"));
        scene.GetSettings()->environment.skyTextureId = skyTextureId;
        scene.GetSettings()->debug.enableInfiniteGrid = true;
        scene.GetSettings()->debug.enableBillboardCameras = false;
        scene.GetSettings()->debug.enableBillboardPointLights = false;
        scene.GetSettings()->debug.enableBillboardSpotLights = false;
        scene.GetSettings()->debug.enableBillboardDirectionalLights = false;

        return true;
    }
}