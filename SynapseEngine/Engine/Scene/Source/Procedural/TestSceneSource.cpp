#include "TestSceneSource.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Component/Rendering/PipelineOverrideComponent.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/Component/Physics/ConvexColliderComponent.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Utils/PathUtils.h"

#include <random>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Syn
{
    bool TestSceneSource::Populate(Scene& scene)
    {
        Registry& registry = SceneInsider::GetRegistry(scene, SceneInsider::GetKey());
        EntityID& sceneCam = SceneInsider::GetSceneCameraEntity(scene, SceneInsider::GetKey());
        EntityID& debugCam = SceneInsider::GetDebugCameraEntity(scene, SceneInsider::GetKey());
        HierarchyManager* hm = scene.GetHierarchyManager();

        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto animationManager = ServiceLocator::Get<AnimationManager>();
        auto materialManager = ServiceLocator::Get<MaterialManager>();

        json config;
        std::string path = PathUtils::GetAbsolutePathString("Engine/Scene/Source/Procedural/test_config.json");
        std::ifstream configFile(path);

        if (configFile.is_open())
        {
            try {
                configFile >> config;
                Syn::Info("Test Scene configuration loaded successfully!");
            }
            catch (const json::parse_error& e) {
                Syn::Error("JSON parsing error: {}. Using default settings.", e.what());
            }
        }
        else
        {
            Syn::Warning("scene_config.json not found, using default settings.");
        }

        const std::string modelPath = "../External/glTF-Sample-Assets/Models/";
        const std::string envPath = "Assets/Engine/Environment/";

        bool spawnSponza = config.value("/environment/spawn_sponza"_json_pointer, true);
        bool spawnBistro = config.value("/environment/spawn_bistro"_json_pointer, false);
        bool spawnFloor = config.value("/environment/spawn_floor"_json_pointer, true);
        bool spawnPbrSponza = config.value("/environment/spawn_pbr_sponza"_json_pointer, true);
        bool spawnMonkey = config.value("/environment/spawn_monkey"_json_pointer, true);
        bool useUniqueMaterials = config.value("/materials/use_unique_materials"_json_pointer, true);

        int sharedMatCount = config.value("/materials/shared_material_count"_json_pointer, 25);

        int charCount = config.value("/entities/animated_characters"_json_pointer, 100);
        int staticGeoCount = config.value("/entities/static_geometry"_json_pointer, 100000);
        int physBoxCount = config.value("/entities/physics_boxes"_json_pointer, 0);
        int physSphereCount = config.value("/entities/physics_spheres"_json_pointer, 0);
        int physCapsuleCount = config.value("/entities/physics_capsules"_json_pointer, 0);

        int dirLightCount = config.value("/lights/directional_count"_json_pointer, 1);
        int pointLightCount = config.value("/lights/point_count"_json_pointer, 50);
        int pointShadowCount = config.value("/lights/point_shadow_count"_json_pointer, 5);
        int spotLightCount = config.value("/lights/spot_count"_json_pointer, 50);
        int spotShadowCount = config.value("/lights/spot_shadow_count"_json_pointer, 5);

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

        auto skyTextureId = ServiceLocator::Get<ImageManager>()->LoadImageSync(PathUtils::GetAbsolutePathString(envPath + "MainScene.hdr"));
        scene.GetSettings()->environment.skyTextureId = skyTextureId;

        EntityID rootCameras = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootCameras);
        registry.GetComponent<TagComponent>(rootCameras).name = "Cameras";
        registry.GetComponent<TagComponent>(rootCameras).tag = "Root";
        registry.AddComponent<TransformComponent>(rootCameras);
        registry.GetPool<TransformComponent>()->SetCategory(rootCameras, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootCameras, StorageCategory::Static);

        EntityID rootEnvironment = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootEnvironment);
        registry.GetComponent<TagComponent>(rootEnvironment).name = "Environment";
        registry.GetComponent<TagComponent>(rootEnvironment).tag = "Root";
        registry.AddComponent<TransformComponent>(rootEnvironment);
        registry.GetPool<TransformComponent>()->SetCategory(rootEnvironment, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootEnvironment, StorageCategory::Static);

        EntityID rootCharacters = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootCharacters);
        registry.GetComponent<TagComponent>(rootCharacters).name = "Characters";
        registry.GetComponent<TagComponent>(rootCharacters).tag = "Root";
        registry.AddComponent<TransformComponent>(rootCharacters);
        registry.GetPool<TransformComponent>()->SetCategory(rootCharacters, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootCharacters, StorageCategory::Static);

        EntityID rootStaticGeo = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootStaticGeo);
        registry.GetComponent<TagComponent>(rootStaticGeo).name = "Static Geometry";
        registry.GetComponent<TagComponent>(rootStaticGeo).tag = "Root";
        registry.AddComponent<TransformComponent>(rootStaticGeo);
        registry.GetPool<TransformComponent>()->SetCategory(rootStaticGeo, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootStaticGeo, StorageCategory::Static);

        EntityID rootPhysics = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootPhysics);
        registry.GetComponent<TagComponent>(rootPhysics).name = "Physics Objects";
        registry.GetComponent<TagComponent>(rootPhysics).tag = "Root";
        registry.AddComponent<TransformComponent>(rootPhysics);
        registry.GetPool<TransformComponent>()->SetCategory(rootPhysics, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootPhysics, StorageCategory::Static);

        EntityID rootLights = scene.CreateEntity();
        registry.AddComponent<TagComponent>(rootLights);
        registry.GetComponent<TagComponent>(rootLights).name = "Lights";
        registry.GetComponent<TagComponent>(rootLights).tag = "Root";
        registry.AddComponent<TransformComponent>(rootLights);
        registry.GetPool<TransformComponent>()->SetCategory(rootLights, StorageCategory::Static);
        registry.GetPool<TagComponent>()->SetCategory(rootLights, StorageCategory::Static);

        // Cameras (Main & Debug)
        {
            sceneCam = scene.CreateEntity();
            registry.AddComponent<TagComponent>(sceneCam);
            registry.GetComponent<TagComponent>(sceneCam).name = "Main Camera";
            registry.GetComponent<TagComponent>(sceneCam).tag = "Camera";
            registry.AddComponent<CameraComponent>(sceneCam);
            registry.AddComponent<TransformComponent>(sceneCam);
            registry.GetPool<CameraComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
            registry.GetPool<TransformComponent>()->SetCategory(sceneCam, StorageCategory::Stream);
            registry.GetPool<TagComponent>()->SetCategory(sceneCam, StorageCategory::Static);
            hm->AttachChild(rootCameras, sceneCam);

            debugCam = scene.CreateEntity();
            registry.AddComponent<TagComponent>(debugCam);
            registry.GetComponent<TagComponent>(debugCam).name = "Debug Camera";
            registry.GetComponent<TagComponent>(debugCam).tag = "Camera";
            registry.AddComponent<CameraComponent>(debugCam);
            registry.AddComponent<TransformComponent>(debugCam);
            registry.GetPool<CameraComponent>()->SetCategory(debugCam, StorageCategory::Stream);
            registry.GetPool<TransformComponent>()->SetCategory(debugCam, StorageCategory::Stream);
            registry.GetPool<TagComponent>()->SetCategory(debugCam, StorageCategory::Static);
            hm->AttachChild(rootCameras, debugCam);
        }

        if (spawnMonkey)
        {
            uint32_t monkeyModelIndex = modelManager->LoadModelAsync(PathUtils::GetAbsolutePathString(modelPath + "../External/glTF-Sample-Assets/Models/Suzanne/glTF/Suzanne.gltf"));

            EntityID monkeyId = scene.CreateEntity();
            registry.AddComponent<TagComponent>(monkeyId);
            registry.AddComponent<MaterialOverrideComponent>(monkeyId);
            registry.AddComponent<PipelineOverrideComponent>(monkeyId);
            registry.GetComponent<TagComponent>(monkeyId).name = "Suzanne_Monkey";
            registry.GetComponent<TagComponent>(monkeyId).tag = "Model";
            registry.AddComponent<TransformComponent>(monkeyId);
            registry.AddComponent<ModelComponent>(monkeyId);

            registry.GetComponent<TransformComponent>(monkeyId).translation = glm::vec3(0.0f, 0.0f, 0.0f);
            registry.GetComponent<TransformComponent>(monkeyId).scale = glm::vec3(5.0f, 5.0f, 5.0f);
            registry.GetComponent<ModelComponent>(monkeyId).modelIndex = monkeyModelIndex;

            registry.GetPool<TransformComponent>()->SetCategory(monkeyId, StorageCategory::Static);
            registry.GetPool<ModelComponent>()->SetCategory(monkeyId, StorageCategory::Static);
            registry.GetPool<MaterialOverrideComponent>()->SetCategory(monkeyId, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(monkeyId, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(monkeyId, StorageCategory::Static);

            hm->AttachChild(rootEnvironment, monkeyId);
        }

        if (spawnSponza)
        {
            uint32_t sponzaId = modelManager->LoadModelAsync(PathUtils::GetAbsolutePathString(modelPath + "Sponza/glTF/Sponza.gltf"));

            EntityID sponzaEntity = scene.CreateEntity();
            registry.AddComponent<TagComponent>(sponzaEntity);
            registry.AddComponent<MaterialOverrideComponent>(sponzaEntity);
            registry.AddComponent<PipelineOverrideComponent>(sponzaEntity);
            registry.GetComponent<TagComponent>(sponzaEntity).name = "Classic_Sponza";
            registry.GetComponent<TagComponent>(sponzaEntity).tag = "Model";
            registry.AddComponent<TransformComponent>(sponzaEntity);
            registry.AddComponent<ModelComponent>(sponzaEntity);
            registry.AddComponent<MeshColliderComponent>(sponzaEntity);
            registry.AddComponent<RigidBodyComponent>(sponzaEntity);

            registry.GetComponent<TransformComponent>(sponzaEntity).translation = glm::vec3(0.0f, 0.0f, 0.0f);
            registry.GetComponent<TransformComponent>(sponzaEntity).scale = glm::vec3(25.0f, 25.0f, 25.0f);
            registry.GetComponent<ModelComponent>(sponzaEntity).modelIndex = sponzaId;
            registry.GetComponent<RigidBodyComponent>(sponzaEntity).motionType = PhysicsMotionType::Static;

            registry.GetPool<TransformComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            registry.GetPool<ModelComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            registry.GetPool<RigidBodyComponent>()->SetCategory(sponzaEntity, StorageCategory::Stream);
            registry.GetPool<MeshColliderComponent>()->SetCategory(sponzaEntity, StorageCategory::Stream);
            registry.GetPool<MaterialOverrideComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);

            hm->AttachChild(rootEnvironment, sponzaEntity);
        }

        if (spawnFloor)
        {
            EntityID floorEntity = scene.CreateEntity();
            registry.AddComponent<TagComponent>(floorEntity);
            registry.GetComponent<TagComponent>(floorEntity).name = "Ground_Floor";
            registry.GetComponent<TagComponent>(floorEntity).tag = "Shape";
            registry.AddComponent<TransformComponent>(floorEntity);
            registry.AddComponent<ModelComponent>(floorEntity);
            registry.AddComponent<RigidBodyComponent>(floorEntity);
            registry.AddComponent<BoxColliderComponent>(floorEntity);
            registry.AddComponent<MaterialOverrideComponent>(floorEntity);
            registry.AddComponent<PipelineOverrideComponent>(floorEntity);

            auto& floorTransform = registry.GetComponent<TransformComponent>(floorEntity);
            floorTransform.translation = glm::vec3(0.0f, -1.0f, 0.0f);
            floorTransform.scale = glm::vec3(500.0f, 1.0f, 500.0f);

            auto& floorModel = registry.GetComponent<ModelComponent>(floorEntity);
            floorModel.modelIndex = modelManager->GetResourceIndex(MeshSourceNames::Cube);

            auto& floorRb = registry.GetComponent<RigidBodyComponent>(floorEntity);
            floorRb.motionType = PhysicsMotionType::Static;

            registry.GetPool<TransformComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<ModelComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<RigidBodyComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<BoxColliderComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<MaterialOverrideComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(floorEntity, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(floorEntity, StorageCategory::Static);

            MaterialInfo floorMatInfo{};
            floorMatInfo.color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
            uint32_t floorMatId = materialManager->LoadMaterialSync("FloorMat", floorMatInfo);
            registry.GetComponent<MaterialOverrideComponent>(floorEntity).materials.push_back(floorMatId);

            hm->AttachChild(rootEnvironment, floorEntity);
        }

        if (charCount > 0)
        {
            uint32_t mutantId = modelManager->LoadModelAsync(PathUtils::GetAbsolutePathString(modelPath + "CesiumMan/glTF/CesiumMan.gltf"));
            uint32_t animationId = animationManager->LoadAnimationAsync(PathUtils::GetAbsolutePathString(modelPath + "CesiumMan/glTF/CesiumMan.gltf"), mutantId);

            // Animated Characters
            for (int i = 0; i < charCount; i++)
            {
                EntityID characterEntity = scene.CreateEntity();
                registry.AddComponent<TagComponent>(characterEntity);
                registry.GetComponent<TagComponent>(characterEntity).name = "Mutant_" + std::to_string(i);
                registry.GetComponent<TagComponent>(characterEntity).tag = "Character";
                registry.AddComponent<TransformComponent>(characterEntity);
                registry.AddComponent<ModelComponent>(characterEntity);
                registry.AddComponent<AnimationComponent>(characterEntity);
                registry.AddComponent<MaterialOverrideComponent>(characterEntity);
                registry.AddComponent<PipelineOverrideComponent>(characterEntity);

                registry.GetComponent<TransformComponent>(characterEntity).translation = glm::vec3((rand() % 400) - 200.0f, 0.0f, (rand() % 400) - 200.0f);
                registry.GetComponent<TransformComponent>(characterEntity).scale = glm::vec3(5.f);
                registry.GetComponent<ModelComponent>(characterEntity).modelIndex = mutantId;

                auto& animComp = registry.GetComponent<AnimationComponent>(characterEntity);
                animComp.animationIndex = animationId;
                animComp.speed = 0.5f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 1.5f;

                registry.GetPool<MaterialOverrideComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry.GetPool<PipelineOverrideComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry.GetPool<TagComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry.GetPool<TransformComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry.GetPool<ModelComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry.GetPool<AnimationComponent>()->SetCategory(characterEntity, StorageCategory::Stream);

                hm->AttachChild(rootCharacters, characterEntity);
            }
        }

        std::vector<uint32_t> sharedMaterialIds;
        if (!useUniqueMaterials) {
            for (int j = 0; j < sharedMatCount; j++) {
                float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                MaterialInfo matInfo{};
                matInfo.color = glm::vec4(r, g, b, 0.1f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.9f));
                matInfo.doubleSided = rand() % 2;
                matInfo.isTransparent = rand() % 2;
                matInfo.emissiveFactor = glm::vec3(r, g, b);
                matInfo.emissiveIntensity = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2;
                matInfo.metallicFactor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                matInfo.roughnessFactor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                matInfo.aoStrength = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                sharedMaterialIds.push_back(materialManager->LoadMaterialSync("SharedMat_" + std::to_string(j), matInfo));
            }
        }

        auto ApplyMaterial = [&](EntityID e, int index) {
            auto& overrideComp = registry.GetComponent<MaterialOverrideComponent>(e);
            if (useUniqueMaterials) {
                float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                MaterialInfo matInfo{};
                matInfo.color = glm::vec4(r, g, b, 1.0f);
                matInfo.emissiveFactor = glm::vec3(r, g, b);
                matInfo.metallicFactor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                matInfo.roughnessFactor = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                overrideComp.materials.push_back(materialManager->LoadMaterialSync("UniqueMat_" + std::to_string(index), matInfo));
            }
            else if (!sharedMaterialIds.empty()) {
                overrideComp.materials.push_back(sharedMaterialIds[rand() % sharedMaterialIds.size()]);
            }
            };

        for (int i = 0; i < staticGeoCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "StaticGeo_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Shape";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<ModelComponent>(e);
            registry.AddComponent<MaterialOverrideComponent>(e);
            registry.AddComponent<PipelineOverrideComponent>(e);

            auto& transform = registry.GetComponent<TransformComponent>(e);
            transform.translation = glm::vec3((rand() % 800) - 400.0f, (rand() % 800) + 5, (rand() % 800) - 400.0f);
            transform.rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);

            registry.GetComponent<ModelComponent>(e).modelIndex = geoIds[rand() % geoIds.size()];

            registry.GetPool<MaterialOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);

            ApplyMaterial(e, i);
            hm->AttachChild(rootStaticGeo, e);
        }

        uint32_t cubeMeshId = modelManager->GetResourceIndex(MeshSourceNames::Cube);
        uint32_t sphereMeshId = modelManager->GetResourceIndex(MeshSourceNames::IcoSphere);
        uint32_t capsuleMeshId = modelManager->GetResourceIndex(MeshSourceNames::Capsule);

        for (int i = 0; i < physBoxCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "PhysicsBox_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Physics";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<ModelComponent>(e);
            registry.AddComponent<RigidBodyComponent>(e);
            registry.AddComponent<BoxColliderComponent>(e);
            registry.AddComponent<MaterialOverrideComponent>(e);
            registry.AddComponent<PipelineOverrideComponent>(e);

            registry.GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
            registry.GetComponent<ModelComponent>(e).modelIndex = cubeMeshId;
            registry.GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

            registry.GetPool<MaterialOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<BoxColliderComponent>()->SetCategory(e, StorageCategory::Static);
            ApplyMaterial(e, staticGeoCount + i);

            hm->AttachChild(rootPhysics, e);
        }

        for (int i = 0; i < physSphereCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "PhysicsSphere_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Physics";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<ModelComponent>(e);
            registry.AddComponent<RigidBodyComponent>(e);
            registry.AddComponent<SphereColliderComponent>(e);
            registry.AddComponent<MaterialOverrideComponent>(e);
            registry.AddComponent<PipelineOverrideComponent>(e);

            registry.GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
            registry.GetComponent<ModelComponent>(e).modelIndex = sphereMeshId;
            registry.GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

            registry.GetPool<MaterialOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<SphereColliderComponent>()->SetCategory(e, StorageCategory::Static);
            ApplyMaterial(e, staticGeoCount + physBoxCount + i);

            hm->AttachChild(rootPhysics, e);
        }

        for (int i = 0; i < physCapsuleCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "PhysicsCapsule_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Physics";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<ModelComponent>(e);
            registry.AddComponent<RigidBodyComponent>(e);
            registry.AddComponent<CapsuleColliderComponent>(e);
            registry.AddComponent<MaterialOverrideComponent>(e);
            registry.AddComponent<PipelineOverrideComponent>(e);

            registry.GetComponent<TransformComponent>(e).translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 400) + 5, (rand() % 400) - 200.0f);
            registry.GetComponent<TransformComponent>(e).rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);

            registry.GetComponent<ModelComponent>(e).modelIndex = capsuleMeshId;
            registry.GetComponent<RigidBodyComponent>(e).motionType = PhysicsMotionType::Dynamic;

            registry.GetPool<MaterialOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<PipelineOverrideComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<RigidBodyComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<CapsuleColliderComponent>()->SetCategory(e, StorageCategory::Static);

            ApplyMaterial(e, staticGeoCount + physBoxCount + physSphereCount + i);

            hm->AttachChild(rootPhysics, e);
        }

        for (int i = 0; i < dirLightCount; ++i) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "DirectionalLight_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Light";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<DirectionLightComponent>(e);

            registry.GetComponent<TransformComponent>(e).rotation = glm::vec3(92.0f, 320.0f, 215.0f);
            auto& light = registry.GetComponent<DirectionLightComponent>(e);
            light.color = glm::vec3(1.0f, 0.95f, 0.85f) * 0.55f;
            light.strength = 5.0f;
            light.useShadow = true;

            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<DirectionLightComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<DirectionLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
            registry.GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);

            hm->AttachChild(rootLights, e);
        }

        for (int i = 0; i < pointLightCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "PointLight_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Light";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<PointLightComponent>(e);

            auto& transform = registry.GetComponent<TransformComponent>(e);
            transform.translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 100), (rand() % 400) - 200.0f);

            auto& light = registry.GetComponent<PointLightComponent>(e);
            light.position = transform.translation;
            light.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX);
            light.radius = 5.0f + (rand() % 50);
            light.strength = 5.0f + (rand() % 25);
            light.useShadow = (i < pointShadowCount);

            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<PointLightComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<PointLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
            registry.GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);

            hm->AttachChild(rootLights, e);
        }

        for (int i = 0; i < spotLightCount; i++) {
            EntityID e = scene.CreateEntity();
            registry.AddComponent<TagComponent>(e);
            registry.GetComponent<TagComponent>(e).name = "SpotLight_" + std::to_string(i);
            registry.GetComponent<TagComponent>(e).tag = "Light";
            registry.AddComponent<TransformComponent>(e);
            registry.AddComponent<SpotLightComponent>(e);

            auto& transform = registry.GetComponent<TransformComponent>(e);
            transform.translation = glm::vec3((rand() % 400) - 200.0f, (rand() % 100), (rand() % 400) - 200.0f);
            transform.rotation = glm::vec3(-45.0f - (rand() % 45), (float)(rand() % 360), 0.0f);

            auto& light = registry.GetComponent<SpotLightComponent>(e);
            light.position = transform.translation;
            light.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX);
            light.range = 25.0f + (rand() % 50);
            light.innerAngle = 15.0f + (rand() % 10);
            light.outerAngle = light.innerAngle + 10.0f + (rand() % 15);
            light.strength = 5.0f + (rand() % 25);
            light.useShadow = (i < spotShadowCount);

            registry.GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<SpotLightComponent>()->SetCategory(e, StorageCategory::Stream);
            registry.GetPool<TagComponent>()->SetCategory(e, StorageCategory::Static);
            registry.GetPool<SpotLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(e);
            registry.GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(e);

            hm->AttachChild(rootLights, e);
        }

        return true;
    }
}