#pragma once
#include "Engine/Scene/Scene.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include <random>

namespace Syn
{
    class TestScene : public Scene
    {
    public:
        TestScene(uint32_t frameCount) : Scene(frameCount)
        {
            auto registry = GetRegistry();

            EntityID cameraEntity = registry->CreateEntity();
            registry->AddComponent<TransformComponent>(cameraEntity);
            registry->AddComponent<CameraComponent>(cameraEntity);

            auto& cam = registry->GetComponent<CameraComponent>(cameraEntity);
            cam.position = glm::vec3(0.0f, 0.0f, 5.0f);
            cam.isMain = true;

            auto transformPool = registry->GetPool<TransformComponent>();

            for (int i = 0; i < 100000; i++)
            {
                EntityID e = registry->CreateEntity();
                registry->AddComponent<TransformComponent>(e);

                auto& transform = registry->GetComponent<TransformComponent>(e);

                transform.translation = glm::vec3(
                    (rand() % 100) - 50.0f,
                    (rand() % 100) - 50.0f,
                    (rand() % 100) - 50.0f
                );

                transformPool->SetCategory(e, StorageCategory::Dynamic);
            }
        }
    };
}