#pragma once
#include "Engine/Scene/Scene.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Component/MaterialOverrideComponent.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Component/AnimationComponent.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Component/SpotLightComponent.h"
#include "Engine/Component/DirectionLightComponent.h"
#include <random>

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

            const std::string basePath = "C:/Users/User/Desktop/Models/";

            uint32_t sponzaId = modelManager->LoadModelAsync(basePath + "Sponza-master/sponza.obj");
            uint32_t bistroId = modelManager->LoadModelAsync(basePath + "Bistro/BistroExterior.fbx");
            uint32_t mutantId = modelManager->LoadModelAsync(basePath + "Monster/Mutant/Mutant.dae");

            std::vector<uint32_t> animationIds;
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Breakdance 1990/Breakdance 1990.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Breakdance Ending 1/Breakdance Ending 1.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Dancing/Dancing.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Hip Hop Dancing/Hip Hop Dancing.dae", mutantId));
            animationIds.push_back(animationManager->LoadAnimationAsync(basePath + "Monster/Hip Hop Dancing_2/Hip Hop Dancing.dae", mutantId));

            std::vector<uint32_t> geoIds;
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Sphere));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Cube));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Quad));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::ScreenQuad));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Cylinder));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Cone));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Capsule));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Hemisphere));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Pyramid));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Grid));
            geoIds.push_back(modelManager->GetResourceIndex(MeshSourceNames::Torus));

            {
                //Main Camera
                EntityID cameraEntity = registry->CreateEntity();
                registry->AddComponent<CameraComponent>(cameraEntity);
                registry->AddComponent<TransformComponent>(cameraEntity);
                registry->GetPool<CameraComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                registry->GetPool<TransformComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                _sceneCameraEntity = cameraEntity;
            }

            {
                //Debug Camera
                EntityID cameraEntity = registry->CreateEntity();
                registry->AddComponent<CameraComponent>(cameraEntity);
                registry->AddComponent<TransformComponent>(cameraEntity);
                registry->GetPool<CameraComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                registry->GetPool<TransformComponent>()->SetCategory(cameraEntity, StorageCategory::Stream);
                _debugCameraEntity = cameraEntity;
            }

            {
                // Sponza
                EntityID sponzaEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(sponzaEntity);
                registry->AddComponent<ModelComponent>(sponzaEntity);

                registry->GetComponent<TransformComponent>(sponzaEntity).translation = glm::vec3(0.0f, 0.0f, 0.0f);
                registry->GetComponent<TransformComponent>(sponzaEntity).scale = glm::vec3(0.2f, 0.2f, 0.2f);
                registry->GetComponent<ModelComponent>(sponzaEntity).modelIndex = sponzaId;

                registry->GetPool<TransformComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            }

            {
                // Bistro
                EntityID bistroEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(bistroEntity);
                registry->AddComponent<ModelComponent>(bistroEntity);

                registry->GetComponent<TransformComponent>(bistroEntity).translation = glm::vec3(50.0f, 0.0f, 0.0f);
                registry->GetComponent<ModelComponent>(bistroEntity).modelIndex = bistroId;

                registry->GetPool<TransformComponent>()->SetCategory(bistroEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(bistroEntity, StorageCategory::Static);
            }

            for (int i = 0; i < 5000; i++)
            {
                // Character
                EntityID characterEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(characterEntity);
                registry->AddComponent<ModelComponent>(characterEntity);
                registry->AddComponent<AnimationComponent>(characterEntity);

                registry->GetComponent<TransformComponent>(characterEntity).translation = glm::vec3(
                    (rand() % 400) - 200.0f,
                    (rand() % 400) - 200.0f,
                    (rand() % 400) - 200.0f
                );

                registry->GetComponent<TransformComponent>(characterEntity).scale = glm::vec3(5.f);
                registry->GetComponent<ModelComponent>(characterEntity).modelIndex = mutantId;

                auto& animComp = registry->GetComponent<AnimationComponent>(characterEntity);
                animComp.animationIndex = animationIds[rand() % animationIds.size()];
                animComp.speed = 0.5f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 1.5f;

                registry->GetPool<TransformComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<AnimationComponent>()->SetCategory(characterEntity, StorageCategory::Stream);
            }
          
            auto materialManager = ServiceLocator::GetMaterialManager();

            // Random Geometry
            for (int i = 0; i < 500000; i++)
            {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);
                registry->AddComponent<ModelComponent>(e);
                registry->AddComponent<MaterialOverrideComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);
                transform.translation = glm::vec3(
                    (rand() % 400) - 200.0f,
                    (rand() % 400) - 200.0f,
                    (rand() % 400) - 200.0f
                );

                auto& model = registry->GetComponent<ModelComponent>(e);
                model.modelIndex = geoIds[rand() % geoIds.size()];

                registry->GetPool<TransformComponent>()->SetCategory(e, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(e, StorageCategory::Static);

                float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                float randomFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float alpha = 0.1f + (randomFloat * 0.9f);

                /*
                MaterialInfo randomMatInfo{};
                randomMatInfo.color = glm::vec4(r, g, b, alpha);
                randomMatInfo.doubleSided = rand() % 2;
                randomMatInfo.isTransparent = rand() % 2;

                randomMatInfo.emissiveFactor = glm::vec3(r, g, b);
                randomMatInfo.emissiveIntensity = randomFloat * 2;

                std::string matName = "RandomGeometryMat_" + std::to_string(i);
                uint32_t randomMatId = materialManager->LoadMaterial(matName, randomMatInfo);

                auto& overrideComp = registry->GetComponent<MaterialOverrideComponent>(e);
                overrideComp.materials.push_back(randomMatId);
                */
            }

            for (int i = 0; i < 1; ++i)
            {
                EntityID dirLightEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(dirLightEntity);
                registry->AddComponent<DirectionLightComponent>(dirLightEntity);

                auto& transform = registry->GetComponent<TransformComponent>(dirLightEntity);
                transform.rotation = glm::vec3(-45.0f, 45.0f, 0.0f);

                auto& light = registry->GetComponent<DirectionLightComponent>(dirLightEntity);
                light.color = glm::vec3(1.0f, 0.95f, 0.85f) * 0.55f;
                light.strength = 5.0f;
                light.useShadow = true;

                registry->GetPool<TransformComponent>()->SetCategory(dirLightEntity, StorageCategory::Stream);
                registry->GetPool<DirectionLightComponent>()->SetCategory(dirLightEntity, StorageCategory::Stream);
                registry->GetPool<DirectionLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(dirLightEntity);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(dirLightEntity);
            }

            for (int i = 0; i < 250; i++)
            {
                EntityID lightEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(lightEntity);
                registry->AddComponent<PointLightComponent>(lightEntity);

                auto& transform = registry->GetComponent<TransformComponent>(lightEntity);
                transform.translation = glm::vec3(
                    (rand() % 400) - 200.0f,
                    (rand() % 50) - 25.0f,
                    (rand() % 400) - 200.0f
                );

                auto& light = registry->GetComponent<PointLightComponent>(lightEntity);
                light.position = transform.translation;
                light.color = glm::vec3(
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
                );
                light.radius = 2 + (rand() % 10);
                light.strength = 5.0f + (rand() % 25);
                light.useShadow = (i < 5);

                registry->GetPool<TransformComponent>()->SetCategory(lightEntity, StorageCategory::Stream);
                registry->GetPool<PointLightComponent>()->SetCategory(lightEntity, StorageCategory::Stream);
                registry->GetPool<PointLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(lightEntity);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(lightEntity);
            }

            for (int i = 0; i < 250; i++)
            {
                EntityID spotLightEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(spotLightEntity);
                registry->AddComponent<SpotLightComponent>(spotLightEntity);

                auto& transform = registry->GetComponent<TransformComponent>(spotLightEntity);
                transform.translation = glm::vec3(
                    (rand() % 400) - 200.0f,
                    (rand() % 50) - 25.0f,
                    (rand() % 400) - 200.0f
                );

                transform.rotation = glm::vec3(
                    -45.0f - (rand() % 45),
                    (float)(rand() % 360),
                    0.0f
                );

                auto& light = registry->GetComponent<SpotLightComponent>(spotLightEntity);
                light.position = transform.translation;

                light.color = glm::vec3(
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX)
                );

                light.range = 30.0f + (rand() % 30);
                light.innerAngle = 15.0f + (rand() % 10);
                light.outerAngle = light.innerAngle + 10.0f + (rand() % 15);
                light.strength = 5.0f + (rand() % 25);
                light.useShadow = (i < 5);

                registry->GetPool<TransformComponent>()->SetCategory(spotLightEntity, StorageCategory::Stream);
                registry->GetPool<SpotLightComponent>()->SetCategory(spotLightEntity, StorageCategory::Stream);
                registry->GetPool<SpotLightComponent>()->SetBit<SHADOW_TOGGLE_BIT>(spotLightEntity);
                registry->GetPool<TransformComponent>()->SetBit<TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>(spotLightEntity);
            }
        }
    };
}