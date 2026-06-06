#include "DirectionLightShadowRenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "DirectionLightShadowSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "DirectionLightCullingSystem.h"
#include <cstring>
#include <vector>

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> DirectionLightShadowRenderSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<RenderSystem>::ID,
			TypeInfo<DirectionLightShadowSystem>::ID,
			TypeInfo<DirectionLightCullingSystem>::ID,
        };
    }

    std::vector<TypeID> DirectionLightShadowRenderSystem::GetWriteDependencies() const
    {
        return { 
            TypeInfo<DirectionLightShadowRenderSystem>::ID
        };
    }

    void DirectionLightShadowRenderSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, drawData]() {
            uint32_t currentMainInstances = drawData->Models.totalAllocatedInstances;

            // Check if the main pass allocated instance count changed
            if (_lastMainAllocatedInstances != currentMainInstances || currentMainInstances == 0) {
                _needsRebuild = true;
                _lastMainAllocatedInstances = currentMainInstances;

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    Info("[DirectionLightShadowRenderSystem] Capacity change detected. Main Instances: {}", currentMainInstances);
                }
            }

            if (_needsRebuild) {
                RebuildShadowBuffers(scene);

                // Ensure the GPU buffers are synced for all frames in flight
                uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
                this->SetFramesToUpload(framesInFlight);
            }
            });
    }

    void DirectionLightShadowRenderSystem::RebuildShadowBuffers(Scene* scene)
    {
        auto drawData = scene->GetSceneDrawData();
        auto& mainGroup = drawData->Models;
        auto& shadowGroup = drawData->DirectionLightShadow;

        // Shadow instances require scaling by the number of active cascades and lights
        uint32_t shadowTotalInstances = mainGroup.totalAllocatedInstances * SHADOW_MULTIPLIER;

        if (shadowTotalInstances > 0 && shadowGroup.instances.Size() < shadowTotalInstances) {
            shadowGroup.instances.Resize(shadowTotalInstances);
        }

        if constexpr  (ENABLE_DEBUG_LOGGING) {
            Info("[DirectionLightShadowRenderSystem] Rebuilding Buffers. Shadow Instances: {}, Traditional Cmds: {}, Meshlet Cmds: {}",
                shadowTotalInstances, mainGroup.activeTraditionalCount, mainGroup.activeMeshletCount);
        }

        if (mainGroup.activeTraditionalCount > 0 && shadowGroup.traditionalCmds.Size() < mainGroup.activeTraditionalCount) {
            shadowGroup.traditionalCmds.Resize(mainGroup.activeTraditionalCount);
        }

        if (mainGroup.activeMeshletCount > 0 && shadowGroup.meshletCmds.Size() < mainGroup.activeMeshletCount) {
            shadowGroup.meshletCmds.Resize(mainGroup.activeMeshletCount);
        }

		shadowGroup.totalCommandCount = mainGroup.activeTraditionalCount + mainGroup.activeMeshletCount;

        // Copy base command blueprints from the main model pass
        if (mainGroup.activeTraditionalCount > 0) {
            std::memcpy(
                shadowGroup.traditionalCmds.Data(),
                mainGroup.traditionalCmds.Data(),
                mainGroup.activeTraditionalCount * sizeof(VkDrawIndirectCommand)
            );
        }

        if (mainGroup.activeMeshletCount > 0) {
            std::memcpy(
                shadowGroup.meshletCmds.Data(),
                mainGroup.meshletCmds.Data(),
                mainGroup.activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT)
            );
        }

        const uint32_t paddingFactor = 16;
        if (mainGroup.activeTraditionalCount > 0 && shadowGroup.paddedTraditionalCounts.Size() < mainGroup.activeTraditionalCount * paddingFactor) {
            shadowGroup.paddedTraditionalCounts.Resize(mainGroup.activeTraditionalCount * paddingFactor);
        }

        if (mainGroup.activeMeshletCount > 0 && shadowGroup.paddedMeshletCounts.Size() < mainGroup.activeMeshletCount * paddingFactor) {
            shadowGroup.paddedMeshletCounts.Resize(mainGroup.activeMeshletCount * paddingFactor);
        }
    }

    void DirectionLightShadowRenderSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (!this->ShouldForceUpload()) return;

            auto drawData = scene->GetSceneDrawData();
            auto& mainGroup = drawData->Models;
            auto& shadowGroup = drawData->DirectionLightShadow;

            uint32_t shadowTotalInstances = mainGroup.totalAllocatedInstances * SHADOW_MULTIPLIER;
            size_t indirectCount = mainGroup.activeTraditionalCount + mainGroup.activeMeshletCount;

            // Update GPU buffer capacities based on the newly calculated requirements
            if (shadowTotalInstances > 0)
                shadowGroup.instanceBuffer.UpdateCapacity(frameIndex, shadowTotalInstances);

            if (indirectCount > 0)
                shadowGroup.indirectBuffer.UpdateCapacity(frameIndex, indirectCount);

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[DirectionLightShadowRenderSystem] GPU Buffers Capacity Updated for Frame {}.", frameIndex);
            }
            });
    }

    void DirectionLightShadowRenderSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::Finish, [this]() {
            _needsRebuild = false;
            this->DecrementFramesToUpload();
            });
    }
}