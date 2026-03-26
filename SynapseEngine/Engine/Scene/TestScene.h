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

            /*
            {
                // Character
                EntityID characterEntity = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(characterEntity);
                registry->AddComponent<ModelComponent>(characterEntity);
                registry->AddComponent<AnimationComponent>(characterEntity);

                registry->GetComponent<TransformComponent>(characterEntity).scale = glm::vec3(5.f);
                registry->GetComponent<ModelComponent>(characterEntity).modelIndex = mutantId;

                auto& animComp = registry->GetComponent<AnimationComponent>(characterEntity);
                animComp.animationIndex = animationIds[1];
                //animComp.speed = 0.5f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 1.5f;

                registry->GetPool<TransformComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<ModelComponent>()->SetCategory(characterEntity, StorageCategory::Static);
                registry->GetPool<AnimationComponent>()->SetCategory(characterEntity, StorageCategory::Stream);
            }
            */

            for (int i = 0; i < 1000; i++)
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
            for (int i = 0; i < 10000; i++)
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

                MaterialInfo randomMatInfo{};
                randomMatInfo.color = glm::vec4(r, g, b, 1.0f);

                std::string matName = "RandomGeometryMat_" + std::to_string(i);
                uint32_t randomMatId = materialManager->LoadMaterial(matName, randomMatInfo);

                auto& overrideComp = registry->GetComponent<MaterialOverrideComponent>(e);
                overrideComp.materials.push_back(randomMatId);
            }
        }
    };
}