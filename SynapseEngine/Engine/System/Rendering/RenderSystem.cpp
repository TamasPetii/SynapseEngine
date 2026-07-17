#include "RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/FrameContext.h"
#include "MaterialSystem.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"

namespace Syn
{
    std::vector<TypeID> RenderSystem::GetReadDependencies() const
    {
        return { 
            TypeInfo<ModelSystem>::ID,
            TypeInfo<MaterialSystem>::ID
        };
    }

    std::vector<TypeID> RenderSystem::GetWriteDependencies() const
    {
        return { TypeInfo<RenderSystem>::ID };
    }

    void RenderSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        auto overridePool = registry->GetPool<MaterialOverrideComponent>();
        if (!pool) return;

        uint32_t totalModels = static_cast<uint32_t>(scene->GetSystemContext().modelSnapshots.size());
        uint32_t currentModelManagerVersion = scene->GetSystemContext().modelManagerVersion;
        uint32_t currentMaterialManagerVersion = scene->GetSystemContext().materialManagerVersion;

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, overridePool, totalModels, currentModelManagerVersion, currentMaterialManagerVersion]() {
            auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;
            auto& matTypeSnapshot = scene->GetSystemContext().materialRenderTypes;
            
            if (_lastModelManagerVersion != currentModelManagerVersion) {
                _needsRebuild = true;
                _lastModelManagerVersion = currentModelManagerVersion;
            }

            if (_lastMaterialManagerVersion != currentMaterialManagerVersion) {
                _needsRebuild = true;
                _lastMaterialManagerVersion = currentMaterialManagerVersion;
            }

            if (!pool->IsStateBitSet<CHANGED_BIT>() && !pool->IsStateBitSet<INDEX_CHANGED_BIT>() && !_needsRebuild)
                return;

            if (totalModels > _modelCapacities.size()) _modelCapacities.resize(totalModels, 0);
            if (totalModels > _entitiesPerModel.size()) _entitiesPerModel.resize(totalModels);
            if (totalModels > _meshMatCapacities.size()) _meshMatCapacities.resize(totalModels);
            for (auto& vec : _entitiesPerModel) vec.clear();

            auto groupFunc = [this, &pool](EntityID entity) {
                uint32_t mId = pool->Get(entity).modelIndex;
                if (mId < _entitiesPerModel.size()) _entitiesPerModel[mId].push_back(entity);
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) groupFunc(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) groupFunc(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) groupFunc(e);

            bool capacityExceeded = false;
            const uint32_t windowSize = 16;

            for (uint32_t modelId = 0; modelId < totalModels; ++modelId)
            {
                if(_entitiesPerModel[modelId].empty() || modelId >= modelSnapshots.size()) continue;

                const auto& snapshot = modelSnapshots[modelId];
                if (snapshot.state != ResourceState::Ready || !snapshot.resource) continue;
                auto model = snapshot.resource;

                uint32_t meshCount = model->cpuData.globalMeshCount;
                if (_meshMatCapacities[modelId].size() < meshCount) {
                    _meshMatCapacities[modelId].resize(meshCount);
                }

                std::vector<MeshMatCapacity> currentCounts(meshCount);
                const auto& defaultMatIndices = model->cpuData.meshMaterialIndices;

                for (EntityID e : _entitiesPerModel[modelId]) {
                    std::span<const uint32_t> overrides;

                    if (overridePool && overridePool->Has(e)) {
                        overrides = overridePool->Get(e).materials;
                    }

                    for (uint32_t m = 0; m < meshCount; ++m) {
                        uint32_t matIdx = defaultMatIndices[m];

                        if (m < overrides.size() && overrides[m] != UINT32_MAX) {
                            matIdx = overrides[m];
                        }

                        MaterialRenderType type = (matIdx < matTypeSnapshot.size()) ? matTypeSnapshot[matIdx] : MaterialRenderType::Opaque1Sided;
                        currentCounts[m].capacities[type]++;
                    }
                }

                uint32_t totalModelCapRequired = 0;
                for (uint32_t m = 0; m < meshCount; ++m) {
                    for (int t = 0; t < MaterialRenderType::Count; ++t) {
                        if (currentCounts[m].capacities[t] > _meshMatCapacities[modelId][m].capacities[t]) {
                            _meshMatCapacities[modelId][m].capacities[t] = currentCounts[m].capacities[t] + windowSize;
                            capacityExceeded = true;
                        }
                    }
                }

                uint32_t maxModelInstances = 0;
                for (uint32_t m = 0; m < meshCount; ++m) {
                    uint32_t meshTotal = 0;

                    for (int t = 0; t < MaterialRenderType::Count; ++t)
                        meshTotal += _meshMatCapacities[modelId][m].capacities[t];

                    if (meshTotal > maxModelInstances)
                        maxModelInstances = meshTotal;
                }

                if (maxModelInstances > _modelCapacities[modelId]) {
                    _modelCapacities[modelId] = maxModelInstances;
                    capacityExceeded = true;
                }
            }

            if (capacityExceeded || _needsRebuild)
            {
                RebuildGlobalBuffers(scene);

                uint32_t framesInFlight = ServiceLocator::Get<FrameContext>()->framesInFlight;
                this->SetFramesToUpload(framesInFlight);
                scene->GetSceneDrawData()->RequestGlobalSync(framesInFlight);
            }
            });
    }

    void RenderSystem::RebuildGlobalBuffers(Scene* scene)
    {
        auto drawData = scene->GetSceneDrawData();
        auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;

        drawData->Models.activeDescriptorCount = 0;

        uint32_t globalInstanceOffset = 0;
        uint32_t totalMaterialIndicesCapacity = 0;
        uint32_t totalMaxMeshletInstances = 0;

        // 1. Pass: Count commands
        uint32_t tradCmdCounts[MaterialRenderType::Count] = { 0 };
        uint32_t meshletCmdCounts[MaterialRenderType::Count] = { 0 };

        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            if (_modelCapacities[modelId] == 0 || modelId >= modelSnapshots.size()) continue;

            const auto& snapshot = modelSnapshots[modelId];
            if (snapshot.state != ResourceState::Ready || !snapshot.resource) continue;
            auto model = snapshot.resource;

            const auto& blueprints = model->cpuData.baseDrawCommands;
            for (size_t i = 0; i < blueprints.size(); ++i) {
                uint32_t meshIndex = static_cast<uint32_t>(i / 4);
                bool isMeshlet = (blueprints[i].isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET);

                for (int t = 0; t < MaterialRenderType::Count; ++t) {
                    if (_meshMatCapacities[modelId][meshIndex].capacities[t] > 0) {
                        if (isMeshlet) 
                            meshletCmdCounts[t]++;
                        else 
                            tradCmdCounts[t]++;
                    }
                }
            }
        }

        // 2. Pass: Distribution of offset ---
        uint32_t tradOffsets[MaterialRenderType::Count];
        uint32_t meshletOffsets[MaterialRenderType::Count];
        drawData->Models.activeTraditionalCount = 0;
        drawData->Models.activeMeshletCount = 0;

        for (int t = 0; t < MaterialRenderType::Count; ++t) {
            drawData->Models.traditionalCmdOffsets[t] = drawData->Models.activeTraditionalCount;
            drawData->Models.traditionalCmdCounts[t] = tradCmdCounts[t];
            tradOffsets[t] = drawData->Models.activeTraditionalCount;
            drawData->Models.activeTraditionalCount += tradCmdCounts[t];

            drawData->Models.meshletCmdOffsets[t] = drawData->Models.activeMeshletCount;
            drawData->Models.meshletCmdCounts[t] = meshletCmdCounts[t];
            meshletOffsets[t] = drawData->Models.activeMeshletCount;
            drawData->Models.activeMeshletCount += meshletCmdCounts[t];
        }

        uint32_t totalDescriptors = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

        uint32_t totalBlueprints = 0;
        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId) {
            if (_modelCapacities[modelId] > 0 && modelId < modelSnapshots.size() && modelSnapshots[modelId].resource) {
                totalBlueprints += static_cast<uint32_t>(modelSnapshots[modelId].resource->cpuData.baseDrawCommands.size());
            }
        }

        if (drawData->Models.traditionalCmds.Size() < drawData->Models.activeTraditionalCount)
            drawData->Models.traditionalCmds.Resize(drawData->Models.activeTraditionalCount);

        if (drawData->Models.meshletCmds.Size() < drawData->Models.activeMeshletCount)
            drawData->Models.meshletCmds.Resize(drawData->Models.activeMeshletCount);

        if (drawData->Models.descriptors.Size() < totalDescriptors)
            drawData->Models.descriptors.Resize(totalDescriptors);

        if (drawData->Models.meshAllocations.Size() < totalBlueprints)
            drawData->Models.meshAllocations.Resize(totalBlueprints);

        if (drawData->Models.modelAllocations.Size() < _modelCapacities.size())
            drawData->Models.modelAllocations.Resize(_modelCapacities.size());

        if (drawData->Debug.modelAabbCmds.Size() < totalDescriptors) {
            drawData->Debug.modelAabbCmds.data.assign(totalDescriptors, drawData->Debug.modelAabbCmdTemplate);
        }

        if (drawData->Debug.modelSphereCmds.Size() < totalDescriptors) {
            drawData->Debug.modelSphereCmds.data.assign(totalDescriptors, drawData->Debug.modelSphereCmdTemplate);
        }

        // 3. Pass: Build Allocations and Descriptors buffers ---
        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            uint32_t capacity = _modelCapacities[modelId];
            if (capacity == 0 || modelId >= modelSnapshots.size()) continue;
            auto model = modelSnapshots[modelId].resource;
            if (!model) continue;

            uint32_t maxMeshletsForModel = 0;
            uint32_t meshCount = model->cpuData.globalMeshCount;
            for (uint32_t m = 0; m < meshCount; ++m) {
                uint32_t lod0Index = m * 4;
                if (lod0Index < model->cpuData.meshletDrawDescriptors.size()) {
                    maxMeshletsForModel += model->cpuData.meshletDrawDescriptors[lod0Index].meshletCount;
                }
            }

            totalMaxMeshletInstances += capacity * maxMeshletsForModel;
            totalMaterialIndicesCapacity += capacity * meshCount;

            const auto& blueprints = model->cpuData.baseDrawCommands;
            ModelAllocationInfo& allocationInfo = drawData->Models.modelAllocations[modelId];
            allocationInfo.maxInstances = capacity;
            allocationInfo.meshAllocationOffset = drawData->Models.activeDescriptorCount;
            allocationInfo.meshAllocationCount = static_cast<uint32_t>(blueprints.size());

            for (size_t i = 0; i < blueprints.size(); ++i)
            {
                uint32_t meshIndex = static_cast<uint32_t>(i / 4);
                const auto& blueprint = blueprints[i];

                MeshAllocationInfo meshAlloc{};
                meshAlloc.descriptorIndex = drawData->Models.activeDescriptorCount;
                meshAlloc.isMeshletPipeline = blueprint.isMeshletPipeline;

                for (int t = 0; t < MaterialRenderType::Count; ++t) {
                    meshAlloc.activeTypes[t] = 0;
                    meshAlloc.indirectIndices[t] = UINT32_MAX;
                }

                for (int type = 0; type < MaterialRenderType::Count; ++type)
                {
                    uint32_t allocatedForThisType = _meshMatCapacities[modelId][meshIndex].capacities[type];
                    if (allocatedForThisType == 0) continue;

                    meshAlloc.activeTypes[type] = 1;
                    meshAlloc.instanceOffsets[type] = globalInstanceOffset;

                    MeshDrawDescriptor desc{};
                    desc.modelIndex = modelId;
                    desc.meshIndex = meshIndex;
                    desc.lodIndex = static_cast<uint32_t>(i % 4);
                    desc.instanceOffset = globalInstanceOffset;
                    desc.maxInstances = allocatedForThisType;
                    desc.isMeshletPipeline = blueprint.isMeshletPipeline;

                    if (blueprint.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET) {
                        uint32_t flatIndirectIdx = meshletOffsets[type]++;
                        meshAlloc.indirectIndices[type] = flatIndirectIdx;

                        uint32_t globalDescIdx = drawData->Models.activeTraditionalCount + flatIndirectIdx;
                        desc.indirectIndex = globalDescIdx;

                        drawData->Models.meshletCmds[flatIndirectIdx] = blueprint.meshletCmd;
                        drawData->Models.descriptors[desc.indirectIndex] = desc;
                    }
                    else {
                        uint32_t flatIndirectIdx = tradOffsets[type]++;
                        meshAlloc.indirectIndices[type] = flatIndirectIdx;

                        uint32_t globalDescIdx = flatIndirectIdx;
                        desc.indirectIndex = globalDescIdx;

                        drawData->Models.traditionalCmds[flatIndirectIdx] = blueprint.traditionalCmd;
                        drawData->Models.descriptors[desc.indirectIndex] = desc;
                    }
                    globalInstanceOffset += allocatedForThisType;
                }

                drawData->Models.meshAllocations[drawData->Models.activeDescriptorCount] = meshAlloc;
                drawData->Models.activeDescriptorCount++;
            }
        }

        drawData->Models.totalAllocatedInstances = globalInstanceOffset;
        drawData->Debug.totalMaxMeshletInstances = totalMaxMeshletInstances;
        drawData->Models.requiredMaterialBufferSize = totalMaterialIndicesCapacity;

        if (drawData->Models.instances.Size() < globalInstanceOffset) {
            drawData->Models.instances.Resize(globalInstanceOffset);
        }

        const uint32_t paddingFactor = 16;

        if (drawData->Models.paddedTraditionalCounts.Size() < drawData->Models.activeTraditionalCount * paddingFactor) {
            drawData->Models.paddedTraditionalCounts.Resize(drawData->Models.activeTraditionalCount * paddingFactor);
        }

        if (drawData->Models.paddedMeshletCounts.Size() < drawData->Models.activeMeshletCount * paddingFactor) {
            drawData->Models.paddedMeshletCounts.Resize(drawData->Models.activeMeshletCount * paddingFactor);
        }

        if (false)
        {
            std::stringstream ss;
            const char* matNames[] = { "Opaque1Sided", "Opaque2Sided", "Transparent1Sided", "Transparent2Sided" };

            ss << "================================================================================\n";
            ss << " RENDER SYSTEM REBUILD REPORT\n";
            ss << "--------------------------------------------------------------------------------\n";
            ss << " Summary:\n";
            ss << "   - Active Descriptors:      " << drawData->Models.activeDescriptorCount << "\n";
            ss << "   - Traditional Commands:    " << drawData->Models.activeTraditionalCount << "\n";
            ss << "   - Meshlet Commands:        " << drawData->Models.activeMeshletCount << "\n";
            ss << "   - Total Allocated Inst:    " << drawData->Models.totalAllocatedInstances << "\n";
            ss << "--------------------------------------------------------------------------------\n";
            ss << " Global Command Breakdowns (Offsets & Counts):\n";

            ss << "   [Traditional Pipeline]\n";
            for (int t = 0; t < MaterialRenderType::Count; ++t) {
                ss << "     - " << matNames[t] << ": Offset = " << drawData->Models.traditionalCmdOffsets[t]
                    << " | Count = " << drawData->Models.traditionalCmdCounts[t] << "\n";
            }

            ss << "   [Meshlet Pipeline]\n";
            for (int t = 0; t < MaterialRenderType::Count; ++t) {
                ss << "     - " << matNames[t] << ": Offset = " << drawData->Models.meshletCmdOffsets[t]
                    << " | Count = " << drawData->Models.meshletCmdCounts[t] << "\n";
            }
            ss << "--------------------------------------------------------------------------------\n";

            for (uint32_t modelId = 0; modelId < drawData->Models.modelAllocations.Size(); ++modelId)
            {
                const auto& mAlloc = drawData->Models.modelAllocations[modelId];
                if (mAlloc.maxInstances == 0) continue;

                ss << " Model [" << modelId << "] - MaxInstances: " << mAlloc.maxInstances
                    << ", MeshOffset: " << mAlloc.meshAllocationOffset
                    << ", MeshCount: " << mAlloc.meshAllocationCount << "\n";

                for (uint32_t i = 0; i < mAlloc.meshAllocationCount; ++i)
                {
                    uint32_t meshAllocIdx = mAlloc.meshAllocationOffset + i;

                    if (meshAllocIdx >= drawData->Models.meshAllocations.Size()) continue;

                    const auto& meshAlloc = drawData->Models.meshAllocations[meshAllocIdx];
                    const auto& desc = drawData->Models.descriptors[meshAlloc.descriptorIndex];

                    ss << "   MeshAlloc [" << i << "] -> DescIdx: " << meshAlloc.descriptorIndex
                        << ", Pipeline: " << (meshAlloc.isMeshletPipeline ? "MESHLET" : "TRADITIONAL") << "\n";

                    for (int t = 0; t < MaterialRenderType::Count; ++t) {
                        if (meshAlloc.activeTypes[t] == 1) {
                            ss << "     -> " << matNames[t]
                                << " | IndirectIdx: " << meshAlloc.indirectIndices[t]
                                << " | InstOffset: " << meshAlloc.instanceOffsets[t] << "\n";
                        }
                    }
                }
            }
            ss << "================================================================================";

            Info("\n{}", ss.str());
        }
    }

    void RenderSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (!this->ShouldForceUpload()) return;

            auto drawData = scene->GetSceneDrawData();

            uint32_t totalInstances = drawData->Models.totalAllocatedInstances;
            if (totalInstances > 0) {
                drawData->Models.instanceBuffer.UpdateCapacity(frameIndex, totalInstances);
            }

            uint32_t totalDescriptors = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
            if (totalDescriptors > 0) {
                drawData->Models.descriptorBuffer.UpdateCapacity(frameIndex, totalDescriptors);
            }

            size_t indirectSize = (drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount);
            if (indirectSize > 0) {
                drawData->Models.indirectBuffer.UpdateCapacity(frameIndex, indirectSize);
                drawData->Debug.modelAabbIndirectBuffer.UpdateCapacity(frameIndex, indirectSize);
                drawData->Debug.modelSphereIndirectBuffer.UpdateCapacity(frameIndex, indirectSize);
            }

            if (drawData->Models.activeDescriptorCount > 0) {
                drawData->Models.meshAllocBuffer.UpdateCapacity(frameIndex, drawData->Models.activeDescriptorCount);
            }

            size_t neededModelCapacity = drawData->Models.modelAllocations.Size();
            if (neededModelCapacity > 0) {
                drawData->Models.modelAllocBuffer.UpdateCapacity(frameIndex, neededModelCapacity);
            }

            size_t totalDescSize = totalDescriptors * sizeof(MeshDrawDescriptor);
            if (totalDescSize > 0)
                drawData->Models.descriptorBuffer.Write(frameIndex, drawData->Models.descriptors.Data(), totalDescSize, 0);

            size_t modelAllocSize = drawData->Models.modelAllocations.Size() * sizeof(ModelAllocationInfo);
            if (modelAllocSize > 0)
                drawData->Models.modelAllocBuffer.Write(frameIndex, drawData->Models.modelAllocations.Data(), modelAllocSize, 0);

            size_t meshAllocSize = drawData->Models.activeDescriptorCount * sizeof(MeshAllocationInfo);
            if (meshAllocSize > 0)
                drawData->Models.meshAllocBuffer.Write(frameIndex, drawData->Models.meshAllocations.Data(), meshAllocSize, 0);

            uint32_t counts[8] = { 0 };
            for (int i = 0; i < MaterialRenderType::Count; ++i) {
                counts[i] = drawData->Models.traditionalCmdCounts[i];
                counts[MaterialRenderType::Count + i] = drawData->Models.meshletCmdCounts[i];
            }
            drawData->Models.drawCountBuffer.Write(frameIndex, counts, sizeof(counts), 0);
            });
    }

    void RenderSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::Finish, [this]() {
            _needsRebuild = false;
            this->DecrementFramesToUpload();
            });
    }
}