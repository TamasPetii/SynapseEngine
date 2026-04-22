#include "AnimationSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Animation/AnimationManager.h"

namespace Syn
{
    std::vector<TypeID> AnimationSystem::GetWriteDependencies() const {
        return { TypeInfo<AnimationSystem>::ID };
    }

    void AnimationSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto animPool = registry->GetPool<AnimationComponent>();
        if (!animPool) return;

        auto animationManager = ServiceLocator::GetAnimationManager();
        auto animations = animationManager->GetResourceSnapshot();

        ParallelForEachIf<UPDATE_BIT>(animPool, subflow, SystemPhaseNames::Update, [animPool, deltaTime, animations](EntityID entity) {
            auto& animComponent = animPool->Get(entity);

            if (animComponent.animationIndex < animations.size())
            {
                const auto& snapshot = animations[animComponent.animationIndex];

                if (snapshot.state == ResourceState::Ready && snapshot.resource)
                {
                    animComponent.isReady = true;

                    auto animData = snapshot.resource;

                    float duration = animData->gpuData.descriptor.durationInSeconds;
                    uint32_t maxFrames = animData->gpuData.descriptor.frameCount;

                    if (duration > 0.0f && maxFrames > 0)
                    {
                        animComponent.time += deltaTime * animComponent.speed;
                        animComponent.time = std::fmod(animComponent.time, duration);

                        if (animComponent.time < 0.0f) {
                            animComponent.time += duration;
                        }

                        float progress = animComponent.time / duration;
                        animComponent.frameIndex = static_cast<uint32_t>(progress * maxFrames);

                        if (animComponent.frameIndex >= maxFrames) {
                            animComponent.frameIndex = maxFrames - 1;
                        }

                        /*
                        Info("Anim Update | Entity: {}, Time: {:.3f}s / {:.3f}s (Progress: {:.1f}%), Frame: {} / {}",
                            entity,
                            animComponent.time,
                            duration,
                            progress * 100.0f,
                            animComponent.frameIndex,
                            maxFrames);
                        */
                    }
                }
                else
                {
                    animComponent.isReady = false;
                }
            }

            animPool->SetBit<CHANGED_BIT>(entity);
            animComponent.version++;
            });
    }

    void AnimationSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto animPool = registry->GetPool<AnimationComponent>();
        if (!animPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::AnimationData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<AnimationComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [animPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& animComponent = animPool->Get(entity);
            auto poolIndex = animPool->GetMapping().Get(entity);

            if (componentBuffer.versions[poolIndex] != animComponent.version)
            {
                componentBuffer.versions[poolIndex] = animComponent.version;

                AnimationComponentGPU gpuComp(animComponent);
                gpuComp.animationIndex = animComponent.isReady ? animComponent.animationIndex : UINT32_MAX;

                bufferHandler[poolIndex] = gpuComp;
            }
            };

        ForEachStream(animPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
            ForEachDynamic(animPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(animPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}