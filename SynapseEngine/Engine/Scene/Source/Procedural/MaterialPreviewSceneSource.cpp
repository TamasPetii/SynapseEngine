#include "MaterialPreviewSceneSource.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Utils/PathUtils.h"

#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>

namespace Syn
{
    bool MaterialPreviewSceneSource::Populate(Scene& scene)
    {
        Registry& registry = SceneInsider::GetRegistry(scene, SceneInsider::GetKey());
        EntityID& sceneCam = SceneInsider::GetSceneCameraEntity(scene, SceneInsider::GetKey());
        HierarchyManager* hm = scene.GetHierarchyManager();

        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        Syn::Info("Populating Material Preview Scene...");

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
        camComp.target = glm::vec3(0.0f);
        camComp.distance = 12.0f;
        camComp.speed = 20.0f;

        registry.GetPool<CameraComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
        registry.GetPool<TransformComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
        hm->AttachChild(rootCameras, sceneCam);

        // Studio lighting setup

        // 1. Key light (warm, casts shadows)
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

        // 2. Fill light (cool, no shadows)
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

        // Base materials
        MaterialInfo floorMatInfo{};
        floorMatInfo.color = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
        floorMatInfo.roughnessFactor = 0.9f;
        uint32_t floorMatId = materialManager->LoadMaterial("Preview_FloorMat", floorMatInfo);

        // Geometries

        // 1. Floor
        EntityID floor = scene.CreateEntity();
        registry.AddComponent<TagComponent>(floor);
        registry.GetComponent<TagComponent>(floor).name = "Studio_Floor";

        registry.AddComponent<TransformComponent>(floor);
        registry.GetComponent<TransformComponent>(floor).translation = glm::vec3(0.0f, -2.0f, 0.0f);
        registry.GetComponent<TransformComponent>(floor).scale = glm::vec3(25.0f, 1.0f, 25.0f);
        registry.GetPool<TransformComponent>()->SetCategory(floor, StorageCategory::Static);

        registry.AddComponent<ModelComponent>(floor);
        registry.GetComponent<ModelComponent>(floor).modelIndex = modelManager->GetResourceIndex(MeshSourceNames::Cube);
        registry.GetPool<ModelComponent>()->SetCategory(floor, StorageCategory::Static);

        registry.AddComponent<MaterialOverrideComponent>(floor);
        registry.GetComponent<MaterialOverrideComponent>(floor).materials.push_back(floorMatId);
        registry.GetPool<MaterialOverrideComponent>()->SetCategory(floor, StorageCategory::Static);

        hm->AttachChild(rootEnvironment, floor);

        // Helper for preview objects
        auto CreatePreviewObject = [&](const std::string& name, uint32_t modelIndex, glm::vec3 pos, glm::vec3 scale = glm::vec3(1.0f)) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = name;
            registry.GetComponent<TagComponent>(e).tag = "Preview";
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);

            registry.AddComponent<TransformComponent>(e);
            registry.GetComponent<TransformComponent>(e).translation = pos;
            registry.GetComponent<TransformComponent>(e).scale = scale;
            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Static);

            registry.AddComponent<ModelComponent>(e);
            registry.GetComponent<ModelComponent>(e).modelIndex = modelIndex;
            registry.GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);

            registry.AddComponent<MaterialOverrideComponent>(e);
            registry.GetPool<MaterialOverrideComponent>()->SetCategory(e, StorageCategory::Static);

            hm->AttachChild(rootEnvironment, e);
            };

        // 2. Center object: Suzanne
        std::string basePath = "C:/Users/User/Desktop/Models/";
        uint32_t monkeyId = modelManager->LoadModelAsync(basePath + "Monkey/monkey.obj");
        CreatePreviewObject("Center_Monkey", monkeyId, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.5f));

        // 3. 8 primitive shapes in a circle
        std::vector<std::pair<std::string, uint32_t>> previewShapes = {
            {"Preview_Sphere", modelManager->GetResourceIndex(MeshSourceNames::Sphere)},
            {"Preview_Cube", modelManager->GetResourceIndex(MeshSourceNames::Cube)},
            {"Preview_Cylinder", modelManager->GetResourceIndex(MeshSourceNames::Cylinder)},
            {"Preview_Torus", modelManager->GetResourceIndex(MeshSourceNames::Torus)},
            {"Preview_Cone", modelManager->GetResourceIndex(MeshSourceNames::Cone)},
            {"Preview_Capsule", modelManager->GetResourceIndex(MeshSourceNames::Capsule)},
            {"Preview_Hemisphere", modelManager->GetResourceIndex(MeshSourceNames::Hemisphere)},
            {"Preview_IcoSphere", modelManager->GetResourceIndex(MeshSourceNames::IcoSphere)}
        };

        float radius = 8.0f;
        for (size_t i = 0; i < previewShapes.size(); ++i)
        {
            float angle = (360.0f / previewShapes.size()) * i;
            float rad = glm::radians(angle);

            float x = glm::cos(rad) * radius;
            float z = glm::sin(rad) * radius;
            float y = 0.5f;

            CreatePreviewObject(previewShapes[i].first, previewShapes[i].second, glm::vec3(x, y, z));
        }

        return true;
    }
}