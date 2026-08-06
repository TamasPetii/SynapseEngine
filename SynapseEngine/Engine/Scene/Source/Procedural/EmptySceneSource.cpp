#include "EmptySceneSource.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    bool EmptySceneSource::Populate(Scene& scene)
    {
        Registry& registry = SceneInsider::GetRegistry(scene, SceneInsider::GetKey());
        EntityID& sceneCam = SceneInsider::GetSceneCameraEntity(scene, SceneInsider::GetKey());
        HierarchyManager* hm = scene.GetHierarchyManager();

        Syn::Info("Populating Empty Scene...");

        EntityID rootCameras = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootCameras);
        registry.GetComponent<TagComponent>(rootCameras).name = "Cameras";
        registry.GetComponent<TagComponent>(rootCameras).tag = "Root";
        registry.AddComponent<TransformComponent>(rootCameras);
        registry.GetPool<TransformComponent>()->SetCategory(rootCameras, StorageCategory::Static);

        sceneCam = scene.CreateEntity();
        registry.AddComponent<TagComponent>(sceneCam);
        registry.GetComponent<TagComponent>(sceneCam).name = "Main Camera";
        registry.GetComponent<TagComponent>(sceneCam).tag = "Camera";

        registry.AddComponent<TransformComponent>(sceneCam);
        auto& camTransform = registry.GetComponent<TransformComponent>(sceneCam);
        camTransform.translation = glm::vec3(0.0f, 0.0f, 5.0f);

        registry.AddComponent<CameraComponent>(sceneCam);
        auto& camComp = registry.GetComponent<CameraComponent>(sceneCam);
        camComp.useOrbit = false;

        registry.GetPool<CameraComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
        registry.GetPool<TransformComponent>()->SetCategory(sceneCam, StorageCategory::Stream);

        hm->AttachChild(rootCameras, sceneCam);

        scene.GetSettings()->debug.enableInfiniteGrid = false;
        scene.GetSettings()->debug.enableBillboardCameras = false;
        scene.GetSettings()->debug.enableBillboardPointLights = false;
        scene.GetSettings()->debug.enableBillboardSpotLights = false;
        scene.GetSettings()->debug.enableBillboardDirectionalLights = false;
        scene.GetSettings()->environment.skyTextureId = 0xFFFFFFFF;

        return true;
    }
}