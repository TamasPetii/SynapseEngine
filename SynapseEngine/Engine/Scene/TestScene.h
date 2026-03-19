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

            uint32_t sponzaId = modelManager->LoadModelAsync("C:/Users/User/Desktop/Models/Sponza-master/sponza.obj");
            uint32_t bistroId = modelManager->LoadModelAsync("C:/Users/User/Desktop/Models/Bistro/BistroExterior.fbx");

            std::vector<uint32_t> geoIds;
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Sphere, []() { return MeshFactory::CreateSphere(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Cube, []() { return MeshFactory::CreateCube(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Quad, []() { return MeshFactory::CreateQuad(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::ScreenQuad, []() { return MeshFactory::CreateScreenQuad(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Cylinder, []() { return MeshFactory::CreateCylinder(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Cone, []() { return MeshFactory::CreateCone(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Capsule, []() { return MeshFactory::CreateCapsule(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Hemisphere, []() { return MeshFactory::CreateHemisphere(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Pyramid, []() { return MeshFactory::CreatePyramid(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Grid, []() { return MeshFactory::CreateGrid(); }));
            geoIds.push_back(modelManager->LoadModelFromStaticMeshAsync(MeshSourceNames::Torus, []() { return MeshFactory::CreateTorus(); }));

            //Camera
            EntityID cameraEntity = registry->CreateEntity();
            registry->AddComponent<CameraComponent>(cameraEntity);
            auto& cam = registry->GetComponent<CameraComponent>(cameraEntity);
            cam.position = glm::vec3(0.0f, 0.0f, 5.0f);
            cam.isMain = true;
            _sceneCameraEntity = cameraEntity;

            // Sponza
            EntityID sponzaEntity = registry->CreateEntity();
            registry->AddComponent<TransformComponent>(sponzaEntity);
            registry->AddComponent<ModelComponent>(sponzaEntity);

            registry->GetComponent<TransformComponent>(sponzaEntity).translation = glm::vec3(0.0f, 0.0f, 0.0f);
            registry->GetComponent<TransformComponent>(sponzaEntity).scale = glm::vec3(0.2f, 0.2f, 0.2f);
            registry->GetComponent<ModelComponent>(sponzaEntity).modelIndex = sponzaId;

            registry->GetPool<TransformComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(sponzaEntity, StorageCategory::Static);

            // Bistro
            EntityID bistroEntity = registry->CreateEntity();
            registry->AddComponent<TransformComponent>(bistroEntity);
            registry->AddComponent<ModelComponent>(bistroEntity);

            registry->GetComponent<TransformComponent>(bistroEntity).translation = glm::vec3(50.0f, 0.0f, 0.0f);
            registry->GetComponent<ModelComponent>(bistroEntity).modelIndex = bistroId;

            registry->GetPool<TransformComponent>()->SetCategory(bistroEntity, StorageCategory::Static);
            registry->GetPool<ModelComponent>()->SetCategory(bistroEntity, StorageCategory::Static);

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