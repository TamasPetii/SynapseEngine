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

#include "PipelineSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/Rendering/PipelineOverrideComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/System/Rendering/PipelineOverrideSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> PipelineSystem::GetReadDependencies() const {
        return {
            TypeInfo<MaterialSystem>::ID
        };
    }

    std::vector<TypeID> PipelineSystem::GetWriteDependencies() const {
        return {
            TypeInfo<PipelineSystem>::ID,
            TypeInfo<PipelineOverrideSystem>::ID
        };
    }

    void PipelineSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        if (!pool) return;

        auto overridePool = registry->GetPool<PipelineOverrideComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        auto drawData = scene->GetSceneDrawData();
        uint32_t currentModelManagerVersion = scene->GetSystemContext().modelManagerVersion;

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, tagPool, currentModelManagerVersion, drawData, overridePool]() {

            bool needsRebuild = false;
            bool needsUpload = false;

            if (_lastModelManagerVersion != currentModelManagerVersion) {
                needsRebuild = true;
                _lastModelManagerVersion = currentModelManagerVersion;
            }

            bool overrideHasChanges = !overridePool->GetDirtyStatics().empty()
                || overridePool->IsStateBitSet<CHANGED_BIT>()
                || overridePool->IsStateBitSet<INDEX_CHANGED_BIT>();

            bool modelHasChanges = !pool->GetDirtyStatics().empty() ||
                pool->IsStateBitSet<CHANGED_BIT>() ||
                pool->IsStateBitSet<INDEX_CHANGED_BIT>();

            bool hasChanges = overrideHasChanges || modelHasChanges || needsRebuild;

            if (!hasChanges) {
                return;
            }

            needsUpload = true;
            auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;

            uint32_t totalExactPipelines = 0;
            auto countFunc = [&](EntityID entity) {
                bool isShared = overridePool && overridePool->Has(entity) && overridePool->Get(entity).sharedPipelineEntity != NULL_ENTITY;
                if (isShared) return;

                auto& comp = pool->Get(entity);
                if (comp.modelIndex == UINT32_MAX) return;

                const auto& snapshot = modelSnapshots[comp.modelIndex];
                if (snapshot.state == ResourceState::Ready && snapshot.resource) {
                    totalExactPipelines += snapshot.resource->cpuData.globalMeshCount;
                }
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) countFunc(e);

            _flatPipelineIndices.resize(totalExactPipelines);
            uint32_t currentOffset = 0;

            auto processEntity = [&](EntityID entity) {
                bool isShared = overridePool && overridePool->Has(entity) && overridePool->Get(entity).sharedPipelineEntity != NULL_ENTITY;
                if (isShared) return;

                auto& comp = pool->Get(entity);
                if (comp.modelIndex >= modelSnapshots.size()) return;

                const auto& snapshot = modelSnapshots[comp.modelIndex];
                if (snapshot.state != ResourceState::Ready || !snapshot.resource) return;
                auto model = snapshot.resource;

                uint32_t meshCount = model->cpuData.globalMeshCount;

                std::span<const uint32_t> overrides;
                if (overridePool && overridePool->Has(entity)) {
                    auto& overrideComp = overridePool->Get(entity);
                    if (overrideComp.pipelines.size() != meshCount) {
                        overrideComp.pipelines.resize(meshCount, UINT32_MAX);
                    }
                    overrides = overrideComp.pipelines;
                }

                comp.pipelineOffset = currentOffset;
                comp.version++;
                pool->SetBit<CHANGED_BIT>(entity);
                pool->MarkStaticDirty(entity);
                needsUpload = true;

                for (uint32_t i = 0; i < meshCount; ++i) {
                    uint32_t lod0Index = i * 4;
                    uint32_t defaultPipe = model->cpuData.baseDrawCommands[lod0Index].pipelineRenderType;

                    uint32_t pipeIdx = defaultPipe;
                    if (!overrides.empty() && i < overrides.size() && overrides[i] != UINT32_MAX) {
                        pipeIdx = overrides[i];
                    }

                    _flatPipelineIndices[currentOffset + i] = pipeIdx;
                }

                currentOffset += meshCount;
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) processEntity(e);

            auto processSharedEntity = [&](EntityID entity) {
                if (!overridePool || !overridePool->Has(entity)) return;

                EntityID sharedEntity = overridePool->Get(entity).sharedPipelineEntity;
                if (sharedEntity == NULL_ENTITY) return;

                if (pool->Has(sharedEntity)) {
                    auto& comp = pool->Get(entity);
                    uint32_t masterOffset = pool->Get(sharedEntity).pipelineOffset;

                    comp.pipelineOffset = masterOffset;
                    comp.version++;
                    pool->SetBit<CHANGED_BIT>(entity);
                    pool->MarkStaticDirty(entity);
                }
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) processSharedEntity(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) processSharedEntity(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) processSharedEntity(e);

            if (needsRebuild || needsUpload) {
                uint32_t framesInFlight = ServiceLocator::Get<FrameContext>()->framesInFlight;
                this->SetFramesToUpload(framesInFlight);
                scene->GetSceneDrawData()->RequestGlobalSync(framesInFlight);
            }
            });
    }

    void PipelineSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            bool force = this->ShouldForceUpload();

            if (force) {
                this->DecrementFramesToUpload();
            }

            if (!force || _flatPipelineIndices.empty())
                return;

            auto drawData = scene->GetSceneDrawData();

            size_t reqFromModels = drawData->Models.requiredMaterialBufferSize;
            size_t actualElements = _flatPipelineIndices.size();
            size_t requiredElements = std::max(actualElements, reqFromModels);

            drawData->Models.pipelineIndexBuffer.UpdateCapacity(frameIndex, requiredElements);

            size_t actualDataSize = _flatPipelineIndices.size() * sizeof(uint32_t);
            drawData->Models.pipelineIndexBuffer.Write(frameIndex, _flatPipelineIndices.data(), actualDataSize, 0);
            });
    }
}