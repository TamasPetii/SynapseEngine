#include "RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ModelManager.h"
#include "Engine/System/ModelSystem.h"

namespace Syn
{
    std::vector<TypeID> RenderSystem::GetReadDependencies() const
    {
        return { TypeInfo<ModelSystem>::ID };
    }

    std::vector<TypeID> RenderSystem::GetWriteDependencies() const
    {
        return { TypeInfo<RenderSystem>::ID };
    }

    void RenderSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        if (!pool) return;

        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t totalModels = static_cast<uint32_t>(modelManager->GetResourceCount());
        uint32_t currentModelManagerVersion = modelManager->GetVersion();

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, totalModels, currentModelManagerVersion]() {
            if (_lastModelManagerVersion != currentModelManagerVersion) {
                _needsRebuild = true;
                _lastModelManagerVersion = currentModelManagerVersion;
            }

            if (!pool->IsStateBitSet<CHANGED_BIT>() && !pool->IsStateBitSet<INDEX_CHANGED_BIT>() && !_needsRebuild)
                return;

            if (totalModels > _modelCapacities.size()) {
                _modelCapacities.resize(totalModels, 0);
            }

            if (totalModels > _currentCounts.size()) {
                _currentCounts.resize(totalModels, 0);
            }

            std::fill(_currentCounts.begin(), _currentCounts.begin() + totalModels, 0);

            auto countFunc = [this, &pool](EntityID entity) {
                _currentCounts[pool->Get(entity).modelIndex]++;
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) countFunc(e);

            bool capacityExceeded = false;

            for (uint32_t modelId = 0; modelId < totalModels; ++modelId)
            {
                uint32_t count = _currentCounts[modelId];
                if (count == 0) continue;

                if (count > _modelCapacities[modelId])
                {
                    capacityExceeded = true;
                    _modelCapacities[modelId] = count + 16;
                }
            }

            if (capacityExceeded || _needsRebuild)
            {
                RebuildGlobalBuffers(scene);

                auto drawData = scene->GetSceneDrawData();
                _framesToUpload = static_cast<uint32_t>(drawData->globalIndirectCommandBuffers.size());
            }
            });
    }

    void RenderSystem::RebuildGlobalBuffers(Scene* scene)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto drawData = scene->GetSceneDrawData();

        drawData->activeDescriptorCount = 0;
        drawData->activeTraditionalCount = 0;
        drawData->activeMeshletCount = 0;

        uint32_t globalInstanceOffset = 0;
        
        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            uint32_t capacity = _modelCapacities[modelId];
            if (capacity == 0) continue;

            auto model = modelManager->GetResource(modelId);
            if (!model) continue;

            const auto& blueprints = model->hardwareBuffers.baseDrawCommands;

            ModelAllocationInfo& allocationInfo = drawData->modelAllocations[modelId];
            allocationInfo.maxInstances = capacity;
            allocationInfo.meshAllocationOffset = drawData->activeDescriptorCount;
            allocationInfo.meshAllocationCount = static_cast<uint32_t>(blueprints.size());

            for (size_t i = 0; i < blueprints.size(); ++i)
            {
                const auto& blueprint = blueprints[i];

                MeshDrawDescriptor desc{};
                desc.modelIndex = modelId;
                desc.meshIndex = static_cast<uint32_t>(i / 4);
                desc.lodIndex = static_cast<uint32_t>(i % 4);
                desc.instanceOffset = globalInstanceOffset;
                desc.maxInstances = capacity;
                desc.isMeshletPipeline = blueprint.isMeshletPipeline;

                MeshAllocationInfo meshAlloc{};
                meshAlloc.descriptorIndex = drawData->activeDescriptorCount;
                meshAlloc.instanceOffset = globalInstanceOffset;
                meshAlloc.isMeshletPipeline = blueprint.isMeshletPipeline;

                if (blueprint.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                {
                    desc.indirectIndex = SceneDrawData::MESHLET_OFFSET_START + drawData->activeMeshletCount;
                    drawData->meshletCommands[drawData->activeMeshletCount] = blueprint.meshletCmd;
                    meshAlloc.indirectIndex = desc.indirectIndex;
                    drawData->drawDescriptors[desc.indirectIndex] = desc;
                    drawData->activeMeshletCount++;
                }
                else
                {
                    desc.indirectIndex = drawData->activeTraditionalCount;
                    drawData->traditionalCommands[drawData->activeTraditionalCount] = blueprint.traditionalCmd;
                    meshAlloc.indirectIndex = desc.indirectIndex;
                    drawData->drawDescriptors[desc.indirectIndex] = desc;
                    drawData->activeTraditionalCount++;
                }

                drawData->meshAllocations[drawData->activeDescriptorCount] = meshAlloc;
                drawData->activeDescriptorCount++;
                globalInstanceOffset += capacity;
            }
        }

        drawData->totalAllocatedInstances = globalInstanceOffset;

        if (false)
        {
            std::stringstream ss;

            ss << "================================================================================\n";
            ss << " RENDER SYSTEM REBUILD REPORT\n";
            ss << "--------------------------------------------------------------------------------\n";
            ss << " Summary:\n";
            ss << "   - Active Descriptors:      " << drawData->activeDescriptorCount << "\n";
            ss << "   - Traditional Commands:    " << drawData->activeTraditionalCount << "\n";
            ss << "   - Meshlet Commands:        " << drawData->activeMeshletCount << "\n";
            ss << "   - Total Allocated Inst:    " << drawData->totalAllocatedInstances << "\n";
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
                        << ", IndirectIdx: " << meshAlloc.indirectIndex
                        << ", InstOffset: " << meshAlloc.instanceOffset
                        << ", Type: " << (meshAlloc.isMeshletPipeline ? "MESHLET" : "TRADITIONAL") << "\n";

                    ss << "     Descriptor -> SubMesh: " << desc.meshIndex
                        << ", LOD: " << desc.lodIndex
                        << ", MaxInst: " << desc.maxInstances
                        << ", GlobalIndirectIdx: " << desc.indirectIndex << "\n";
                }
            }
            ss << "================================================================================";

            Info("\n{}", ss.str());
        }
    }

    void RenderSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (_framesToUpload == 0) return;

            auto drawData = scene->GetSceneDrawData();

            size_t tradDescSize = drawData->activeTraditionalCount * sizeof(MeshDrawDescriptor);
            if (tradDescSize > 0)
                drawData->globalIndirectCommandDescriptorBuffers[frameIndex]->Write(drawData->drawDescriptors.data(), tradDescSize, 0);

            size_t meshletDescSize = drawData->activeMeshletCount * sizeof(MeshDrawDescriptor);
            if (meshletDescSize > 0) {
                size_t meshletDescGpuOffset = SceneDrawData::MESHLET_OFFSET_START * sizeof(MeshDrawDescriptor);
                drawData->globalIndirectCommandDescriptorBuffers[frameIndex]->Write(
                    drawData->drawDescriptors.data() + SceneDrawData::MESHLET_OFFSET_START,
                    meshletDescSize,
                    meshletDescGpuOffset
                );
            }

            size_t modelAllocSize = drawData->activeDescriptorCount * sizeof(ModelAllocationInfo);
            if (modelAllocSize > 0)
                drawData->globalModelAllocationBuffers[frameIndex]->Write(drawData->modelAllocations.data(), modelAllocSize, 0);

            size_t meshAllocSize = drawData->activeDescriptorCount * sizeof(MeshAllocationInfo);
            if (meshAllocSize > 0)
                drawData->globalMeshAllocationBuffers[frameIndex]->Write(drawData->meshAllocations.data(), meshAllocSize, 0);

            uint32_t counts[2] = { drawData->activeTraditionalCount, drawData->activeMeshletCount };
            drawData->globalDrawCountBuffers[frameIndex]->Write(counts, sizeof(counts), 0);
            });
    }

    void RenderSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::Finish, [this]() {
            _needsRebuild = false;

            if (_framesToUpload > 0) {
                _framesToUpload--;
            }
            });
    }
}