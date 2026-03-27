#include "RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameCOntext.h"
#include "MaterialSystem.h"
#include "Engine/Component/MaterialOverrideComponent.h"

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

        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        uint32_t totalModels = static_cast<uint32_t>(modelManager->GetResourceCount());
        uint32_t currentModelManagerVersion = modelManager->GetVersion();
        uint32_t currentMaterialManagerVersion = materialManager->GetVersion();

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, overridePool, modelManager, materialManager, totalModels, currentModelManagerVersion, currentMaterialManagerVersion]() {
            auto modelSnapshots = modelManager->GetResourceSnapshot();
            auto matTypeSnapshot = materialManager->GetRenderTypeSnapshot();
            
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

                auto model = modelSnapshots[modelId].resource;
                if (!model) continue;

                uint32_t meshCount = model->gpuData.globalMeshCount;
                if (_meshMatCapacities[modelId].size() < meshCount) {
                    _meshMatCapacities[modelId].resize(meshCount);
                }

                std::vector<MeshMatCapacity> currentCounts(meshCount);
                const auto& defaultMatIndices = model->meshMaterialIndices;

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
                this->SetFramesToUpload(ServiceLocator::GetFrameContext()->framesInFlight);
            }
            });
    }

    void RenderSystem::RebuildGlobalBuffers(Scene* scene)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto drawData = scene->GetSceneDrawData();
        auto modelSnapshots = modelManager->GetResourceSnapshot();

        drawData->activeDescriptorCount = 0;

        uint32_t globalInstanceOffset = 0;
        uint32_t totalMaterialIndicesCapacity = 0;
        uint32_t totalMaxMeshletInstances = 0;

        // --- 1. Pass: Számoljuk össze a szükséges command darabszámokat ---
        uint32_t tradCmdCounts[MaterialRenderType::Count] = { 0 };
        uint32_t meshletCmdCounts[MaterialRenderType::Count] = { 0 };

        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            if (_modelCapacities[modelId] == 0 || modelId >= modelSnapshots.size()) continue;
            auto model = modelSnapshots[modelId].resource;
            if (!model) continue;

            const auto& blueprints = model->baseDrawCommands;
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

        // --- 2. Pass: Osztjuk ki az offseteket ---
        uint32_t tradOffsets[MaterialRenderType::Count];
        uint32_t meshletOffsets[MaterialRenderType::Count];
        drawData->activeTraditionalCount = 0;
        drawData->activeMeshletCount = 0;

        for (int t = 0; t < MaterialRenderType::Count; ++t) {
            drawData->traditionalCmdOffsets[t] = drawData->activeTraditionalCount;
            drawData->traditionalCmdCounts[t] = tradCmdCounts[t];
            tradOffsets[t] = drawData->activeTraditionalCount;
            drawData->activeTraditionalCount += tradCmdCounts[t];

            drawData->meshletCmdOffsets[t] = drawData->activeMeshletCount;
            drawData->meshletCmdCounts[t] = meshletCmdCounts[t];
            meshletOffsets[t] = drawData->activeMeshletCount;
            drawData->activeMeshletCount += meshletCmdCounts[t];
        }

        // --- 3. Pass: Építsük fel az Allocations és Descriptors buffereket ---
        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            uint32_t capacity = _modelCapacities[modelId];
            if (capacity == 0 || modelId >= modelSnapshots.size()) continue;
            auto model = modelSnapshots[modelId].resource;
            if (!model) continue;

            uint32_t maxMeshletsForModel = 0;
            uint32_t meshCount = model->gpuData.globalMeshCount;
            for (uint32_t m = 0; m < meshCount; ++m) {
                uint32_t lod0Index = m * 4;
                if (lod0Index < model->gpuData.meshletData.drawDescriptors.size()) {
                    maxMeshletsForModel += model->gpuData.meshletData.drawDescriptors[lod0Index].meshletCount;
                }
            }

            totalMaxMeshletInstances += capacity * maxMeshletsForModel;
            totalMaterialIndicesCapacity += capacity * meshCount;

            const auto& blueprints = model->baseDrawCommands;
            ModelAllocationInfo& allocationInfo = drawData->modelAllocations[modelId];
            allocationInfo.maxInstances = capacity;
            allocationInfo.meshAllocationOffset = drawData->activeDescriptorCount;
            allocationInfo.meshAllocationCount = static_cast<uint32_t>(blueprints.size());

            for (size_t i = 0; i < blueprints.size(); ++i)
            {
                uint32_t meshIndex = static_cast<uint32_t>(i / 4);
                const auto& blueprint = blueprints[i];

                MeshAllocationInfo meshAlloc{};
                meshAlloc.descriptorIndex = drawData->activeDescriptorCount;
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

                        uint32_t globalDescIdx = drawData->activeTraditionalCount + flatIndirectIdx;
                        desc.indirectIndex = globalDescIdx;

                        drawData->meshletCommands[flatIndirectIdx] = blueprint.meshletCmd;
                        drawData->drawDescriptors[desc.indirectIndex] = desc;
                    }
                    else {
                        uint32_t flatIndirectIdx = tradOffsets[type]++;
                        meshAlloc.indirectIndices[type] = flatIndirectIdx;

                        uint32_t globalDescIdx = flatIndirectIdx;
                        desc.indirectIndex = globalDescIdx;

                        drawData->traditionalCommands[flatIndirectIdx] = blueprint.traditionalCmd;
                        drawData->drawDescriptors[desc.indirectIndex] = desc;
                    }
                    globalInstanceOffset += allocatedForThisType;
                }

                drawData->meshAllocations[drawData->activeDescriptorCount] = meshAlloc;
                drawData->activeDescriptorCount++;
            }
        }

        drawData->totalAllocatedInstances = globalInstanceOffset;
        drawData->totalMaxMeshletInstances = totalMaxMeshletInstances;
        drawData->requiredMaterialBufferSize = totalMaterialIndicesCapacity * sizeof(uint32_t);

        if (true)
        {
            std::stringstream ss;
            const char* matNames[] = { "Opaque1Sided", "Opaque2Sided", "Transparent1Sided", "Transparent2Sided" };

            ss << "================================================================================\n";
            ss << " RENDER SYSTEM REBUILD REPORT\n";
            ss << "--------------------------------------------------------------------------------\n";
            ss << " Summary:\n";
            ss << "   - Active Descriptors:      " << drawData->activeDescriptorCount << "\n";
            ss << "   - Traditional Commands:    " << drawData->activeTraditionalCount << "\n";
            ss << "   - Meshlet Commands:        " << drawData->activeMeshletCount << "\n";
            ss << "   - Total Allocated Inst:    " << drawData->totalAllocatedInstances << "\n";
            ss << "--------------------------------------------------------------------------------\n";
            ss << " Global Command Breakdowns (Offsets & Counts):\n";

            ss << "   [Traditional Pipeline]\n";
            for (int t = 0; t < MaterialRenderType::Count; ++t) {
                ss << "     - " << matNames[t] << ": Offset = " << drawData->traditionalCmdOffsets[t]
                    << " | Count = " << drawData->traditionalCmdCounts[t] << "\n";
            }

            ss << "   [Meshlet Pipeline]\n";
            for (int t = 0; t < MaterialRenderType::Count; ++t) {
                ss << "     - " << matNames[t] << ": Offset = " << drawData->meshletCmdOffsets[t]
                    << " | Count = " << drawData->meshletCmdCounts[t] << "\n";
            }
            ss << "--------------------------------------------------------------------------------\n";

            for (uint32_t modelId = 0; modelId < drawData->modelAllocations.size(); ++modelId)
            {
                const auto& mAlloc = drawData->modelAllocations[modelId];
                if (mAlloc.maxInstances == 0) continue;

                ss << " Model [" << modelId << "] - MaxInstances: " << mAlloc.maxInstances
                    << ", MeshOffset: " << mAlloc.meshAllocationOffset
                    << ", MeshCount: " << mAlloc.meshAllocationCount << "\n";

                for (uint32_t i = 0; i < mAlloc.meshAllocationCount; ++i)
                {
                    uint32_t meshAllocIdx = mAlloc.meshAllocationOffset + i;

                    if (meshAllocIdx >= drawData->meshAllocations.size()) continue;

                    const auto& meshAlloc = drawData->meshAllocations[meshAllocIdx];
                    const auto& desc = drawData->drawDescriptors[meshAlloc.descriptorIndex];

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

            size_t requiredDataSize = drawData->totalMaxMeshletInstances * sizeof(DebugMeshletInstance);
            auto& currentDebugBuffer = drawData->debugInstanceBuffers[frameIndex];

            if (false && requiredDataSize > 0 && currentDebugBuffer->GetSize() < requiredDataSize)
            {
                size_t oldSize = currentDebugBuffer->GetSize();
                Info("RenderSystem: [BUFFER REALLOCATION] Reallocating debug instance buffer for frame {}! Old size: {} bytes, New size: {} bytes", frameIndex, oldSize, requiredDataSize);

                currentDebugBuffer = Vk::BufferFactory::CreatePersistent(
                    requiredDataSize,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
            }

            uint32_t totalDescriptors = drawData->activeTraditionalCount + drawData->activeMeshletCount;
            size_t totalDescSize = totalDescriptors * sizeof(MeshDrawDescriptor);

            if (totalDescSize > 0)
                drawData->globalIndirectCommandDescriptorBuffers[frameIndex]->Write(drawData->drawDescriptors.data(), totalDescSize, 0);

            size_t modelAllocSize = drawData->modelAllocations.size() * sizeof(ModelAllocationInfo);
            if (modelAllocSize > 0)
                drawData->globalModelAllocationBuffers[frameIndex]->Write(drawData->modelAllocations.data(), modelAllocSize, 0);

            size_t meshAllocSize = drawData->activeDescriptorCount * sizeof(MeshAllocationInfo);
            if (meshAllocSize > 0)
                drawData->globalMeshAllocationBuffers[frameIndex]->Write(drawData->meshAllocations.data(), meshAllocSize, 0);

            uint32_t counts[8] = { 0 };
            for (int i = 0; i < MaterialRenderType::Count; ++i) {
                counts[i] = drawData->traditionalCmdCounts[i];
                counts[MaterialRenderType::Count + i] = drawData->meshletCmdCounts[i];
            }

            drawData->globalDrawCountBuffers[frameIndex]->Write(counts, sizeof(counts), 0);
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