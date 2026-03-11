#include "RenderSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ModelManager.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Component/ModelComponent.h"

namespace Syn
{
    std::vector<TypeID> RenderSystem::GetReadDependencies() const
    {
        return { TypeInfo<ModelComponent>::ID };
    }

    RenderSystem::RenderSystem()
    {
        _descriptors.resize(MAX_INDIRECT_COMMANDS);
        _traditionalCommands.resize(MESHLET_OFFSET_START, { 0,0,0,0 });
        _meshletCommands.resize(MAX_INDIRECT_COMMANDS - MESHLET_OFFSET_START, { 0,0,0 });
    }

    const ModelAllocationInfo* RenderSystem::GetModelAllocation(uint32_t modelId) const
    {
        if (modelId < _modelAllocations.size())
            return &_modelAllocations[modelId];
        return nullptr;
    }

    void RenderSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        if (!pool) return;

        // Lekérjük a modellek számát a Taskflow lambda ELŐTT
        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t totalModels = static_cast<uint32_t>(modelManager->GetResourceCount());

        subflow.emplace([this, scene, pool, totalModels]() {
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
                    _modelCapacities[modelId] = count + 256;
                }
            }

            if (capacityExceeded || _needsRebuild)
            {
                RebuildGlobalBuffers(scene);
                _needsUpload = true;
            }
            }).name("RenderSystem Update");
    }

    void RenderSystem::RebuildGlobalBuffers(Scene* scene)
    {
        auto modelManager = ServiceLocator::GetModelManager();

        _activeDescriptorCount = 0;
        _activeTraditionalCount = 0;
        _activeMeshletCount = 0;

        uint32_t globalInstanceOffset = 0;

        if (_modelCapacities.size() > _modelAllocations.size())
            _modelAllocations.resize(_modelCapacities.size());

        for (uint32_t modelId = 0; modelId < _modelCapacities.size(); ++modelId)
        {
            uint32_t capacity = _modelCapacities[modelId];
            if (capacity == 0) continue;

            //Mutex miatt lehet nagyon lassú?
            auto model = modelManager->GetResource(modelId);
            if (!model) continue;

            const auto& blueprints = model->hardwareBuffers.baseDrawCommands;

            ModelAllocationInfo& allocationInfo = _modelAllocations[modelId];
            allocationInfo.maxInstances = capacity;
            allocationInfo.meshAllocations.clear();
            allocationInfo.meshAllocations.reserve(blueprints.size());

            for (size_t i = 0; i < blueprints.size(); ++i)
            {
                const auto& blueprint = blueprints[i];

                uint32_t meshIdx = static_cast<uint32_t>(i / 4);
                uint32_t lodIdx = static_cast<uint32_t>(i % 4);

                MeshDrawDescriptor& desc = _descriptors[_activeDescriptorCount];
                desc.modelIndex = modelId;
                desc.meshIndex = meshIdx;
                desc.lodIndex = lodIdx;
                desc.instanceOffset = globalInstanceOffset;
                desc.maxInstances = capacity;
                desc.isMeshletPipeline = blueprint.isMeshletPipeline;

                MeshAllocationInfo meshAlloc{};
                meshAlloc.descriptorIndex = _activeDescriptorCount;
                meshAlloc.instanceOffset = globalInstanceOffset;
                meshAlloc.isMeshletPipeline = blueprint.isMeshletPipeline;

                if (blueprint.isMeshletPipeline == MeshDrawBlueprint::PIPELINE_MESHLET)
                {
                    desc.indirectIndex = MESHLET_OFFSET_START + _activeMeshletCount;
                    _meshletCommands[_activeMeshletCount] = blueprint.meshletCmd;
                    meshAlloc.indirectIndex = desc.indirectIndex;
                    _activeMeshletCount++;
                }
                else
                {
                    desc.indirectIndex = _activeTraditionalCount;
                    _traditionalCommands[_activeTraditionalCount] = blueprint.traditionalCmd;
                    meshAlloc.indirectIndex = desc.indirectIndex;
                    _activeTraditionalCount++;
                }

                allocationInfo.meshAllocations.push_back(meshAlloc);

                _activeDescriptorCount++;
                globalInstanceOffset += capacity;
            }
        }
    }

    void RenderSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        subflow.emplace([this, scene]() {
            if (!_needsUpload) return;

            size_t descSize = _activeDescriptorCount * sizeof(MeshDrawDescriptor);
            scene->GetGlobalDescriptorBuffer()->Write(_descriptors.data(), descSize);

            size_t tradSize = _activeTraditionalCount * sizeof(VkDrawIndirectCommand);
            scene->GetGlobalIndirectCommandBuffer()->Write(_traditionalCommands.data(), tradSize);

            size_t meshletSize = _activeMeshletCount * sizeof(VkDrawMeshTasksIndirectCommandEXT);
            size_t meshletGpuOffset = MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
            scene->GetGlobalIndirectCommandBuffer()->Write(_meshletCommands.data(), meshletSize, meshletGpuOffset);

            uint32_t counts[2] = { _activeTraditionalCount, _activeMeshletCount };
            scene->GetGlobalDrawCountBuffer()->Write(counts, sizeof(counts));
            }).name("RenderSystem GPU Upload");
    }

    void RenderSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        subflow.emplace([this]() {
            _needsRebuild = false;
            _needsUpload = false;
            }).name("RenderSystem Finish");
    }
}