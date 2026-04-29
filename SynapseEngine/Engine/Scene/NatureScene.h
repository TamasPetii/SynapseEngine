#pragma once
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/Logger/Logger.h" 

#include <random>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Syn
{
    class NatureScene : public Scene
    {
    public:
        NatureScene(uint32_t frameCount) : Scene(frameCount)
        {
            auto registry = GetRegistry();
            auto modelManager = ServiceLocator::GetModelManager();
            auto animationManager = ServiceLocator::GetAnimationManager();
            auto materialManager = ServiceLocator::GetMaterialManager();

            json config;
            std::ifstream configFile("../Engine/Scene/nature_config.json");
            if (configFile.is_open())
            {
                try {
                    configFile >> config;
                    Syn::Info("Scene configuration loaded successfully!");
                }
                catch (const json::parse_error& e) {
                    Syn::Error("JSON parsing error: {}. Using default settings.", e.what());
                }
            }
            else
            {
                Syn::Warning(" nature_config.json not found, using default settings.");
            }

            std::string basePath = config.value("/paths/base_model_path"_json_pointer, "C:/Users/User/Desktop/Models/");
            bool spawnFloor = config.value("/environment/spawn_floor"_json_pointer, true);
            int staticGeoCount = config.value("/entities/static_geometry"_json_pointer, 100);

            int dirLightCount = config.value("/lights/directional_count"_json_pointer, 1);
            int pointLightCount = config.value("/lights/point_count"_json_pointer, 10);
            int pointShadowCount = config.value("/lights/point_shadow_count"_json_pointer, 5);
            int spotLightCount = config.value("/lights/spot_count"_json_pointer, 10);
            int spotShadowCount = config.value("/lights/spot_shadow_count"_json_pointer, 5);

            std::vector<uint32_t> treeIds = {
                modelManager->LoadModelAsync(basePath + "Nature/Tree/CommonTree_3.fbx"),
                modelManager->LoadModelAsync(basePath + "Nature/Tree-aVOxaHRPWe/CommonTree_2.fbx"),
                modelManager->LoadModelAsync(basePath + "Nature/Tree-qZtx0AHhcy/CommonTree_1.fbx"),
                modelManager->LoadModelAsync(basePath + "Nature/Tree-t9KbsfYdXz/CommonTree_5.fbx"),
                modelManager->LoadModelAsync(basePath + "Nature/Tree-YWjGDJ9F7g/CommonTree_4.fbx")
            };

            // Cameras (Main & Debug)
            {
                EntityID cameraEntity = registry->CreateEntity();
                registry->AddComponent<CameraComponent>(cameraEntity);
                registry->AddComponent<TransformComponent>(cameraEntity);
                registry->GetPool<CameraComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                registry->GetPool<TransformComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                _sceneCameraEntity = cameraEntity;

                EntityID debugCam = registry->CreateEntity();
                registry->AddComponent<CameraComponent>(debugCam);
                registry->AddComponent<TransformComponent>(debugCam);
                registry->GetPool<CameraComponent>()->SetCategory(debugCam, StorageCategory::Stream);
                registry->GetPool<TransformComponent>()->SetCategory(debugCam, StorageCategory::Stream);
                _debugCameraEntity = debugCam;
            }

            if (spawnFloor)
            {
                EntityID floorEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(floorEntity);
                registry->AddComponent<ModelComponent>(floorEntity);
                registry->AddComponent<RigidBodyComponent>(floorEntity);
                registry->AddComponent<BoxColliderComponent>(floorEntity);
                registry->AddComponent<MaterialOverrideComponent>(floorEntity);

                auto& floorTransform = registry->GetComponent<TransformComponent>(floorEntity);
                floorTransform.translation = glm::vec3(0.0f, -1.0f, 0.0f);
                floorTransform.scale = glm::vec3(500.0f, 1.0f, 500.0f);

                auto& floorModel = registry->GetComponent<ModelComponent>(floorEntity);
                floorModel.modelIndex = modelManager->GetResourceIndex(MeshSourceNames::Cube);

                auto& floorRb = registry->GetComponent<RigidBodyComponent>(floorEntity);
                floorRb.motionType = PhysicsMotionType::Static;

                registry->GetPool<TransformComponent>()->SetCategory(floorEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(floorEntity, StorageCategory::Static);
                registry->GetPool<RigidBodyComponent>()->SetCategory(floorEntity, StorageCategory::Static);
                registry->GetPool<BoxColliderComponent>()->SetCategory(floorEntity, StorageCategory::Static);

                MaterialInfo floorMatInfo{};
                floorMatInfo.color = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
                uint32_t floorMatId = materialManager->LoadMaterial("FloorMat", floorMatInfo);
                registry->GetComponent<MaterialOverrideComponent>(floorEntity).materials.push_back(floorMatId);
            }

         
            // Static Geometry
            for (int i = 0; i < staticGeoCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);
                transform.translation = glm::vec3((rand() % 1000) - 500.0f, 0, (rand() % 1000) - 500.0f);
                transform.rotation = glm::vec3(0, rand() % 360, 0);
                transform.scale = glm::vec3(0.01f);

                registry->GetComponent<ModelComponent>(e).modelIndex = treeIds[rand() % treeIds.size()];

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
            }

            // Lights: Directional
            for (int i = 0; i < dirLightCount; ++i) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<DirectionLightComponent>(e);

                registry->GetComponent<TransformComponent>(e).rotation = glm::vec3(-45.0f, 45.0f, 0.0f);
                auto& light = registry->GetComponent<DirectionLightComponent>(e);
                light.color = glm::vec3(1.0f, 0.95f, 0.85f) * 0.55f;
                light.strength = 5.0f;
                light.useShadow = true;

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<DirectionLightComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<DirectionLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);
            }

            // Lights: Point
            for (int i = 0; i < pointLightCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<PointLightComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);
                transform.translation = glm::vec3((rand() % 1000) - 500.0f, (rand() % 10) + 5.0f, (rand() % 1000) - 500.0f);

                auto& light = registry->GetComponent<PointLightComponent>(e);
                light.position = transform.translation;
                light.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX);
                light.radius = 2 + (rand() % 10);
                light.strength = 5.0f + (rand() % 25);
                light.useShadow = (i < pointShadowCount);

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<PointLightComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<PointLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);
            }

            // Lights: Spot
            for (int i = 0; i < spotLightCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<SpotLightComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);
                transform.translation = glm::vec3((rand() % 1000) - 500.0f, (rand() % 10) + 5.0f, (rand() % 1000) - 500.0f);
                transform.rotation = glm::vec3(-45.0f - (rand() % 45), (float)(rand() % 360), 0.0f);

                auto& light = registry->GetComponent<SpotLightComponent>(e);
                light.position = transform.translation;
                light.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX);
                light.range = 30.0f + (rand() % 30);
                light.innerAngle = 15.0f + (rand() % 10);
                light.outerAngle = light.innerAngle + 10.0f + (rand() % 15);
                light.strength = 5.0f + (rand() % 25);
                light.useShadow = (i < spotShadowCount);

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<SpotLightComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<SpotLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);
            }
        }
    };
}