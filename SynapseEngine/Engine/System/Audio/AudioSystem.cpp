#include "AudioSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Audio/AudioListenerComponent.h"
#include "Engine/Component/Audio/AudioSourceComponent.h"
#include "Engine/Audio/Engine/IAudioEngine.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/ServiceLocator.h"

namespace Syn
{
    std::vector<TypeID> AudioSystem::GetReadDependencies() const
    {
        return { 
            TypeInfo<TransformSystem>::ID,
            TypeInfo<CameraSystem>::ID
        };
    }

    std::vector<TypeID> AudioSystem::GetWriteDependencies() const
    {
        return {};
    }

    void AudioSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto listenerPool = registry->GetPool<AudioListenerComponent>();
        auto sourcePool = registry->GetPool<AudioSourceComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();

        auto audioEngine = ServiceLocator::Get<IAudioEngine>();

        if (!audioEngine) return;

        tf::Task listenerTask = this->EmplaceTask(subflow, "UpdateListener", [registry, transformPool, listenerPool, cameraPool, audioEngine]() {
            if (!listenerPool || !transformPool) return;

            for (auto entity : listenerPool->GetStorage().GetDenseEntities()) {
                if (listenerPool->Get(entity).active && transformPool->Has(entity)) {
                    const auto& tr = transformPool->Get(entity);

                    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

                    if (cameraPool && cameraPool->Has(entity)) {
                        const auto& cam = cameraPool->Get(entity);
                        forward = cam.direction;
                        up = cam.up;
                    }
                    else {
                        float yaw = glm::radians(tr.rotation.y);
                        float pitch = glm::radians(tr.rotation.x);
                        forward = glm::vec3(
                            glm::cos(yaw) * glm::cos(pitch),
                            glm::sin(pitch),
                            glm::sin(yaw) * glm::cos(pitch)
                        );
                        forward = glm::normalize(forward);
                        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
                        up = glm::normalize(glm::cross(right, forward));
                    }

                    audioEngine->SetListenerTransform(tr.translation, forward, up);
                    break;
                }
            }
            });

        if (!sourcePool || !transformPool) return;

        auto sourceEntities = sourcePool->GetStorage().GetDenseEntities();

        const auto& systemContext = scene->GetSystemContext();
        const auto* audioSnapshots = &systemContext.soundSnapshots;

        std::optional<tf::Task> sourceTask = this->ForEach(sourceEntities, subflow, "UpdateSources",
            [transformPool, sourcePool, audioEngine, audioSnapshots](EntityID entity) {
                const auto& source = sourcePool->Get(entity);

                if (source.soundIndex == UINT32_MAX || source.soundIndex >= audioSnapshots->size()) return;

                const auto& snapshot = (*audioSnapshots)[source.soundIndex];

                if (snapshot.state != ResourceState::Ready || !snapshot.resource) return;

                glm::vec3 position{ 0.0f };
                if (transformPool->Has(entity)) {
                    position = transformPool->Get(entity).translation;
                }

                audioEngine->UpdateSound(entity, source, snapshot.resource->cpuData, position);
            });

        if (sourceTask) {
            sourceTask.value().succeed(listenerTask);
        }
    }

    void AudioSystem::OnEntityDestroyed(Scene* scene, EntityID entity)
    {
        auto audioEngine = ServiceLocator::Get<IAudioEngine>();
        if (!audioEngine) return;

        auto registry = scene->GetRegistry();

        if (registry->HasComponent<AudioSourceComponent>(entity))
        {
            audioEngine->StopSound(entity);
        }
    }

    void AudioSystem::OnClean(Scene* scene)
    {
        auto audioEngine = ServiceLocator::Get<IAudioEngine>();
        if (!audioEngine) return;

        audioEngine->StopAllSounds();
    }
}