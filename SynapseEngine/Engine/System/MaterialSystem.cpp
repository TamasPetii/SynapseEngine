#include "MaterialSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/MaterialOverrideComponent.h"

namespace Syn
{
    std::vector<TypeID> MaterialSystem::GetWriteDependencies() const {
        return { TypeInfo<MaterialSystem>::ID };
    }

    void MaterialSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        if (!pool) return;

        auto overridePool = registry->GetPool<MaterialOverrideComponent>();

        auto drawData = scene->GetSceneDrawData();
        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t currentModelManagerVersion = modelManager->GetVersion();

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, modelManager, currentModelManagerVersion, drawData, overridePool]() {

            bool needsRebuild = false;
            if (_lastModelManagerVersion != currentModelManagerVersion) {
                needsRebuild = true;
                _lastModelManagerVersion = currentModelManagerVersion;
            }

            if (!pool->IsStateBitSet<CHANGED_BIT>() && !pool->IsStateBitSet<INDEX_CHANGED_BIT>() && !needsRebuild) {
                return;
            }

            auto modelSnapshots = modelManager->GetResourceSnapshot();

            uint32_t totalExactMaterials = 0;
            auto countFunc = [&](EntityID entity) {
                auto& comp = pool->Get(entity);
                if (comp.modelIndex < modelSnapshots.size() && modelSnapshots[comp.modelIndex].resource) {
                    totalExactMaterials += static_cast<uint32_t>(modelSnapshots[comp.modelIndex].resource->meshMaterialIndices.size());
                }
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) countFunc(e);

            Info("MaterialSystem: Rebuilding material indices. Total materials expected: {}", totalExactMaterials);

            _flatMaterialIndices.resize(totalExactMaterials);
            uint32_t currentOffset = 0;

            auto processEntity = [&](EntityID entity) {
                auto& comp = pool->Get(entity);
                if (comp.modelIndex >= modelSnapshots.size()) return;

                auto model = modelSnapshots[comp.modelIndex].resource;
                if (!model) return;

                const auto& defaultMaterials = model->meshMaterialIndices;
                uint32_t materialCount = static_cast<uint32_t>(defaultMaterials.size());

                std::span<const uint32_t> overrides;
                if (overridePool && overridePool->Has(entity)) {
                    auto& overrideComp = overridePool->Get(entity);

                    if (overrideComp.materials.size() != materialCount) {
                        overrideComp.materials.resize(materialCount, UINT32_MAX);
                    }

                    overrides = overrideComp.materials;
                }

                if (comp.materialOffset != currentOffset) {
                    comp.materialOffset = currentOffset;
                    comp.version++;
                    pool->SetBit<CHANGED_BIT>(entity);
                }

                Info("MaterialSystem: Entity {} assigned offset: {}, material count: {}", (uint32_t)entity, currentOffset, materialCount);

                for (uint32_t i = 0; i < materialCount; ++i) {
                    if (!overrides.empty() && overrides[i] != UINT32_MAX) {
                        _flatMaterialIndices[currentOffset + i] = overrides[i];
                    }
                    else {
                        _flatMaterialIndices[currentOffset + i] = defaultMaterials[i];
                    }
                }

                currentOffset += materialCount;
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) processEntity(e);

            _framesToUpload = static_cast<uint32_t>(drawData->globalMaterialIndexBuffers.size());
            });
    }

    void MaterialSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (_framesToUpload == 0 || _flatMaterialIndices.empty()) return;

            auto drawData = scene->GetSceneDrawData();

            size_t actualDataSize = _flatMaterialIndices.size() * sizeof(uint32_t);
            size_t bufferSizeToAllocate = std::max(actualDataSize, drawData->requiredMaterialBufferSize);

            auto& currentBuffer = drawData->globalMaterialIndexBuffers[frameIndex];

            if (!currentBuffer || currentBuffer->GetSize() < bufferSizeToAllocate)
            {
                Info("MaterialSystem: [RUG PULL WARNING] Reallocating GPU buffer for frame {}! Old size: {}, New size: {}",
                 frameIndex,
                 currentBuffer ? currentBuffer->GetSize() : 0,
                 bufferSizeToAllocate);

                currentBuffer = Vk::BufferFactory::CreatePersistent(
                    bufferSizeToAllocate,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                );
            }

            currentBuffer->Write(_flatMaterialIndices.data(), actualDataSize, 0);

            if (_framesToUpload > 0) {
                _framesToUpload--;
            }
            });
    }
}