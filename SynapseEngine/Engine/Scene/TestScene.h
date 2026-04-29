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
    class TestScene : public Scene
    {
    public:
        TestScene(uint32_t frameCount) : Scene(frameCount)
        {
            auto registry = GetRegistry();
            auto modelManager = ServiceLocator::GetModelManager();
            auto animationManager = ServiceLocator::GetAnimationManager();
            auto materialManager = ServiceLocator::GetMaterialManager();

            json config;
            std::ifstream configFile("../Engine/Scene/scene_config.json");
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
                Syn::Warning("scene_config.json not found, using default settings.");
            }

            std::string basePath = config.value("/paths/base_model_path"_json_pointer, "C:/Users/User/Desktop/Models/");
            bool spawnSponza = config.value("/environment/spawn_sponza"_json_pointer, true);
            bool spawnBistro = config.value("/environment/spawn_bistro"_json_pointer, false);
            bool spawnFloor = config.value("/environment/spawn_floor"_json_pointer, true);
            bool useUniqueMaterials = config.value("/materials/use_unique_materials"_json_pointer, true);
            int sharedMatCount = config.value("/materials/shared_material_count"_json_pointer, 25);

            int charCount = config.value("/entities/animated_characters"_json_pointer, 100);
            int staticGeoCount = config.value("/entities/static_geometry"_json_pointer, 1000000);
            int physBoxCount = config.value("/entities/physics_boxes"_json_pointer, 0);
            int physSphereCount = config.value("/entities/physics_spheres"_json_pointer, 0);
            int physCapsuleCount = config.value("/entities/physics_capsules"_json_pointer, 0);

            int dirLightCount = config.value("/lights/directional_count"_json_pointer, 1);
            int pointLightCount = config.value("/lights/point_count"_json_pointer, 50);
            int pointShadowCount = config.value("/lights/point_shadow_count"_json_pointer, 5);
            int spotLightCount = config.value("/lights/spot_count"_json_pointer, 50);
            int spotShadowCount = config.value("/lights/spot_shadow_count"_json_pointer, 5);

            uint32_t sponzaId = modelManager->LoadModelAsync(basePath + "Sponza/sponza.obj");
			uint32_t bistroId = modelManager->LoadModelAsync(basePath + "Bistro/BistroExterior.fbx");
            uint32_t mutantId = modelManager->LoadModelAsync(basePath + "Monster/Mutant/Mutant.dae");

            std::vector<uint32_t> animationIds;
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Breakdance 1990/Breakdance 1990.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Breakdance Ending 1/Breakdance Ending 1.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Dancing/Dancing.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Hip Hop Dancing/Hip Hop Dancing.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Hip Hop Dancing_2/Hip Hop Dancing.dae", mutantId));

            std::vector<uint32_t> geoIds = {
                modelManager->GetResourceIndex(MeshSourceNames::Cube),
                modelManager->GetResourceIndex(MeshSourceNames::Sphere),
                modelManager->GetResourceIndex(MeshSourceNames::Quad),
                modelManager->GetResourceIndex(MeshSourceNames::ScreenQuad),
                modelManager->GetResourceIndex(MeshSourceNames::Cylinder),
                modelManager->GetResourceIndex(MeshSourceNames::Cone),
                modelManager->GetResourceIndex(MeshSourceNames::Capsule),
                modelManager->GetResourceIndex(MeshSourceNames::Hemisphere),
                modelManager->GetResourceIndex(MeshSourceNames::Pyramid),
                modelManager->GetResourceIndex(MeshSourceNames::Grid),
                modelManager->GetResourceIndex(MeshSourceNames::Torus)
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

            // Sponza Environment
            if (spawnSponza)
            {
                EntityID sponzaEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(sponzaEntity);
                registry->AddComponent<ModelComponent>(sponzaEntity);

                registry->GetComponent<TransformComponent>(sponzaEntity).translation = glm::vec3(0.0f, 0.0f, 0.0f);
                registry->GetComponent<TransformComponent>(sponzaEntity).scale = glm::vec3(0.2f, 0.2f, 0.2f);
                registry->GetComponent<ModelComponent>(sponzaEntity).modelIndex = sponzaId;

                registry->GetPool<TransformComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            }

            if (spawnBistro)
            {
                EntityID bistroEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(bistroEntity);
                registry->AddComponent<ModelComponent>(bistroEntity);

                registry->GetComponent<TransformComponent>(bistroEntity).translation = glm::vec3(0.0f, 0.0f, 0.0f);
                registry->GetComponent<TransformComponent>(bistroEntity).scale = glm::vec3(0.2f, 0.2f, 0.2f);
                registry->GetComponent<ModelComponent>(bistroEntity).modelIndex = bistroId;

                registry->GetPool<TransformComponent>()->SetCategory(bistroEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(bistroEntity, StorageCategory::Static);
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
                floorMatInfo.color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
                uint32_t floorMatId = materialManager->LoadMaterial("FloorMat", floorMatInfo);
                registry->GetComponent<MaterialOverrideComponent>(floorEntity).materials.push_back(floorMatId);
            }

            // Animated Characters
            for (int i = 0; i < charCount; i++)
            {
                EntityID characterEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(characterEntity);
                registry->AddComponent<ModelComponent>(characterEntity);
                registry->AddComponent<AnimationComponent>(characterEntity);

                registry->GetComponent<TransformComponent>(characterEntity).translation = glm::vec3(
                    (rand() % 400) - 200.0f, (rand() % 400) - 200.0f, (rand() % 400) - 200.0f);
                registry->GetComponent<TransformComponent>(characterEntity).scale = glm::vec3(5.f);
                registry->GetComponent<ModelComponent>(characterEntity).modelIndex = mutantId;

                auto& animComp = registry->GetComponent<AnimationComponent>(characterEntity);
                animComp.animationIndex = animationIds[rand() % animationIds.size()];
                animComp.speed = 0.5f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 1.5f;

                registry->GetPool<TransformComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<AnimationComponent>()->SetCategory(characterEntity, StorageCategory::Stream);
            }

            // Generate Shared Materials
            std::vector<uint32_t> sharedMaterialIds;
            if (!useUniqueMaterials) {
                for (int j = 0; j < sharedMatCount; j++) {
                    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    float randomFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                    /*
                    MaterialInfo matInfo{};
                    matInfo.color = glm::vec4(r, g, b, 0.1f + (randomFloat * 0.9f));
                    matInfo.doubleSided = rand() % 2;
                    matInfo.isTransparent = rand() % 2;
                    matInfo.emissiveFactor = glm::vec3(r, g, b);
                    matInfo.emissiveIntensity = randomFloat * 2;
                    */

                    MaterialInfo matInfo{};
                    matInfo.color = glm::vec4(r, g, b, 1);
                    matInfo.doubleSided = false;
                    matInfo.isTransparent = false;
                    matInfo.emissiveFactor = glm::vec3(1);
                    matInfo.emissiveIntensity = 1;

                    sharedMaterialIds.push_back(materialManager->LoadMaterial("SharedMat_" + std::to_string(j), matInfo));
                }
            }

            auto ApplyMaterial = [&](EntityID e, int index) {
                auto& overrideComp = registry->GetComponent<MaterialOverrideComponent>(e);
                if (useUniqueMaterials) {
                    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                    MaterialInfo matInfo{};
                    matInfo.color = glm::vec4(r, g, b, 1.0f);
                    matInfo.emissiveFactor = glm::vec3(r, g, b);
                    overrideComp.materials.push_back(materialManager->LoadMaterial("UniqueMat_" + std::to_string(index), matInfo));
                }
                else if (!sharedMaterialIds.empty()) {
                    overrideComp.materials.push_back(sharedMaterialIds[rand() % sharedMaterialIds.size()]);
                }
                };

            // Static Geometry
            for (int i = 0; i < staticGeoCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);
                transform.translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
                transform.rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);

                registry->GetComponent<ModelComponent>(e).modelIndex = geoIds[rand() % geoIds.size()];

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);

                ApplyMaterial(e, i);
            }

            uint32_t cubeMeshId = modelManager->GetResourceIndex(MeshSourceNames::Cube);
            uint32_t sphereMeshId = modelManager->GetResourceIndex(MeshSourceNames::Sphere);
            uint32_t capsuleMeshId = modelManager->GetResourceIndex(MeshSourceNames::Capsule);

            // Physics Boxes
            for (int i = 0; i < physBoxCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<RigidBodyComponent>(e);
                registry->AddComponent<BoxColliderComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                registry->GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
                registry->GetComponent<ModelComponent>(e).modelIndex = cubeMeshId;
                registry->GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<BoxColliderComponent>()->SetCategory(e, StorageCategory::Static);
                ApplyMaterial(e, staticGeoCount + i);
            }

            // Physics Spheres
            for (int i = 0; i < physSphereCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<RigidBodyComponent>(e);
                registry->AddComponent<SphereColliderComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                registry->GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
                registry->GetComponent<ModelComponent>(e).modelIndex = sphereMeshId;
                registry->GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<SphereColliderComponent>()->SetCategory(e, StorageCategory::Static);
                ApplyMaterial(e, staticGeoCount + physBoxCount + i);
            }

            // Physics Capsules
            for (int i = 0; i < physCapsuleCount; i++) {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<RigidBodyComponent>(e);
                registry->AddComponent<CapsuleColliderComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                registry->GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);

                // Using rotation for capsules can make falling more interesting!
                registry->GetComponent<TransformComponent>(e).rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);

                registry->GetComponent<ModelComponent>(e).modelIndex = capsuleMeshId;
                registry->GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<CapsuleColliderComponent>()->SetCategory(e, StorageCategory::Static);

                ApplyMaterial(e, staticGeoCount + physBoxCount + physSphereCount + i);
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
                transform.translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 50) - 25.0f, (rand() % 400) - 200.0f);

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
                transform.translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 50) - 25.0f, (rand() % 400) - 200.0f);
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