// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "AnimationSystem.h"
#include "Engine/ServiceLocator.h"

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

        const auto& ctx = scene->GetSystemContext();
        auto& animations = ctx.animationSnapshots;

        ParallelForEachIf<UPDATE_BIT>(animPool, subflow, SystemPhaseNames::Update, [animPool, deltaTime, animations](EntityID entity) {
            auto& animComponent = animPool->Get(entity);

            if (animComponent.animationIndex < animations.size())
            {
                const auto& snapshot = animations[animComponent.animationIndex];

                if (snapshot.state == ResourceState::Ready && snapshot.resource)
                {
                    animComponent.isReady = true;

                    auto animData = snapshot.resource;

                    float duration = animData->cpuData.descriptor.durationInSeconds;
                    uint32_t maxFrames = animData->cpuData.descriptor.frameCount;

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