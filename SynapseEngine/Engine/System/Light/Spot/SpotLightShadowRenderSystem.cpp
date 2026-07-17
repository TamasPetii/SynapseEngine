#include "SpotLightShadowRenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "SpotLightShadowSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "SpotLightCullingSystem.h"
#include <cstring>
#include <vector>

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;
    
    std::vector<TypeID> SpotLightShadowRenderSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<RenderSystem>::ID,
            TypeInfo<SpotLightShadowSystem>::ID,
            TypeInfo<SpotLightCullingSystem>::ID,
        };
    }

    std::vector<TypeID> SpotLightShadowRenderSystem::GetWriteDependencies() const
    {
        return { 
            TypeInfo<SpotLightShadowRenderSystem>::ID
        };
    }

    void SpotLightShadowRenderSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, drawData]() {
            uint32_t currentMainInstances = drawData->Models.totalAllocatedInstances;
            uint32_t currentCommandCount = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

            // Check if the main pass allocated instance count changed
            if (_lastMainAllocatedInstances != currentMainInstances ||
                _lastCommandCount != currentCommandCount || 
                currentMainInstances == 0) 
            {
                _needsRebuild = true;
                _lastMainAllocatedInstances = currentMainInstances;
                _lastCommandCount = currentCommandCount;

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    Info("[SpotLightShadowRenderSystem] Capacity change detected. Main Instances: {}", currentMainInstances);
                }
            }

            if (_needsRebuild) {
                RebuildShadowBuffers(scene);

                uint32_t framesInFlight = ServiceLocator::Get<FrameContext>()->framesInFlight;
                this->SetFramesToUpload(framesInFlight);
            }
        });
    }

    void SpotLightShadowRenderSystem::RebuildShadowBuffers(Scene* scene)
    {
        auto drawData = scene->GetSceneDrawData();
        auto& mainGroup = drawData->Models;
        auto& shadowGroup = drawData->SpotLightShadow;

        uint32_t shadowTotalInstances = mainGroup.totalAllocatedInstances * SPOT_SHADOW_MULTIPLIER;
        if (shadowTotalInstances > 0 && shadowGroup.instances.Size() < shadowTotalInstances) {
            shadowGroup.instances.Resize(shadowTotalInstances);
        }

        if constexpr  (ENABLE_DEBUG_LOGGING) {
            Info("[SpotLightShadowRenderSystem] Rebuilding Buffers. Shadow Instances: {}, Traditional Cmds: {}, Meshlet Cmds: {}",
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

        if (shadowGroup.totalCommandCount > 0) {
            if (shadowGroup.shadowDescriptors.Size() < shadowGroup.totalCommandCount) {
                shadowGroup.shadowDescriptors.Resize(shadowGroup.totalCommandCount);
            }

            std::memcpy(
                shadowGroup.shadowDescriptors.Data(),
                mainGroup.descriptors.Data(),
                shadowGroup.totalCommandCount * sizeof(MeshDrawDescriptor)
            );
        }
    }

    void SpotLightShadowRenderSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (!this->ShouldForceUpload()) return;

            auto drawData = scene->GetSceneDrawData();
            auto& mainGroup = drawData->Models;
            auto& shadowGroup = drawData->SpotLightShadow;

            uint32_t shadowTotalInstances = mainGroup.totalAllocatedInstances * SPOT_SHADOW_MULTIPLIER;
            size_t indirectCount = mainGroup.activeTraditionalCount + mainGroup.activeMeshletCount;

            if (shadowTotalInstances > 0)
            {
                shadowGroup.instanceBuffer.UpdateCapacity(frameIndex, shadowTotalInstances);
                shadowGroup.drawCallKeyBuffer.UpdateCapacity(frameIndex, shadowTotalInstances);
                shadowGroup.sortValuesBuffer.UpdateCapacity(frameIndex, shadowTotalInstances);
                shadowGroup.unsortedInstanceBuffer.UpdateCapacity(frameIndex, shadowTotalInstances);
            }

            if (indirectCount > 0)
            {
                shadowGroup.indirectBuffer.UpdateCapacity(frameIndex, indirectCount);
                shadowGroup.descriptorBuffer.UpdateCapacity(frameIndex, indirectCount);
            }

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[SpotLightShadowRenderSystem] GPU Buffers Capacity Updated for Frame {}.", frameIndex);
            }
        });
    }

    void SpotLightShadowRenderSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::Finish, [this]() {
            _needsRebuild = false;
            this->DecrementFramesToUpload();
        });
    }
}