#include "MaterialSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "MaterialOverrideSystem.h"

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> MaterialSystem::GetWriteDependencies() const {
        return { 
            TypeInfo<MaterialSystem>::ID,
            TypeInfo<MaterialOverrideSystem>::ID
        };
    }

    void MaterialSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<ModelComponent>();
        if (!pool) return;

        auto overridePool = registry->GetPool<MaterialOverrideComponent>();
        auto tagPool = registry->GetPool<TagComponent>();

        auto drawData = scene->GetSceneDrawData();
        uint32_t currentModelManagerVersion = scene->GetSystemContext().modelManagerVersion;

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, pool, tagPool, currentModelManagerVersion, drawData, overridePool]() {

            bool needsRebuild = false;
            bool needsUpload = false;

            if (_lastModelManagerVersion != currentModelManagerVersion) {
                needsRebuild = true;
                _lastModelManagerVersion = currentModelManagerVersion;

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    Info("[MaterialSystem UPDATE] Frame {}: ModelManager version changed to {}", scene->GetSystemContext().frameIndex, currentModelManagerVersion);
                }

            }

            bool materialOverrideHasChanges = !overridePool->GetDirtyStatics().empty()
                                            || overridePool->IsStateBitSet<CHANGED_BIT>()
                                            || overridePool->IsStateBitSet<INDEX_CHANGED_BIT>();

            bool modelHasChanges = !pool->GetDirtyStatics().empty() ||
                              pool->IsStateBitSet<CHANGED_BIT>() ||
                              pool->IsStateBitSet<INDEX_CHANGED_BIT>();

            bool hasChanges = materialOverrideHasChanges || modelHasChanges || needsRebuild;

            if (!hasChanges) {
                return;
            }

            needsUpload = true;
            auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;

            uint32_t totalExactMaterials = 0;
            auto countFunc = [&](EntityID entity) {
                bool isShared = overridePool && overridePool->Has(entity) && overridePool->Get(entity).sharedMaterialEntity != NULL_ENTITY;
                if (isShared) return;

                auto& comp = pool->Get(entity);
                const auto& snapshot = modelSnapshots[comp.modelIndex];
                if (snapshot.state == ResourceState::Ready && snapshot.resource) {
                    totalExactMaterials += static_cast<uint32_t>(snapshot.resource->cpuData.meshMaterialIndices.size());
                }
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) countFunc(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) countFunc(e);

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[MaterialSystem UPDATE] Frame {}: Rebuilding material indices. Total slots expected: {}", scene->GetSystemContext().frameIndex, totalExactMaterials);
            }
            
            _flatMaterialIndices.resize(totalExactMaterials);
            uint32_t currentOffset = 0;

            auto processEntity = [&](EntityID entity) {
                bool isShared = overridePool && overridePool->Has(entity) && overridePool->Get(entity).sharedMaterialEntity != NULL_ENTITY;
                if (isShared) return;

                auto& comp = pool->Get(entity);
                if (comp.modelIndex >= modelSnapshots.size()) return;

                const auto& snapshot = modelSnapshots[comp.modelIndex];
                if (snapshot.state != ResourceState::Ready || !snapshot.resource) return;
                auto model = snapshot.resource;

                const auto& defaultMaterials = model->cpuData.meshMaterialIndices;
                uint32_t materialCount = static_cast<uint32_t>(defaultMaterials.size());
                
                std::span<const uint32_t> overrides;
                if (overridePool && overridePool->Has(entity)) {
                    auto& overrideComp = overridePool->Get(entity);

                    if (overrideComp.materials.size() != materialCount) {
                        overrideComp.materials.resize(materialCount, UINT32_MAX);
                    }

                    overrides = overrideComp.materials;
                }

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    std::string entityName = "Unknown";
                    if (tagPool && tagPool->Has(entity)) entityName = tagPool->Get(entity).name;
                    Info("[MaterialSystem UPDATE] Frame {}: Entity {} data changed! OldOffset: {}, NewOffset: {}, WasReady: {}", scene->GetSystemContext().frameIndex, entityName, comp.materialOffset, currentOffset);
                }

                comp.materialOffset = currentOffset;
                comp.version++;
                pool->SetBit<CHANGED_BIT>(entity);
                pool->MarkStaticDirty(entity);

                needsUpload = true;

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    std::string entityName = "Unknown";
                    if (tagPool && tagPool->Has(entity)) entityName = tagPool->Get(entity).name;
                    Info("MaterialSystem: Entity {} assigned offset: {}, material count: {}", entityName, currentOffset, materialCount);
                }

                for (uint32_t i = 0; i < materialCount; ++i) {
                    uint32_t matIdx = defaultMaterials[i];
                    if (!overrides.empty() && i < overrides.size() && overrides[i] != UINT32_MAX) {
                        matIdx = overrides[i];
                    }
                    
                    if constexpr (ENABLE_DEBUG_LOGGING) {
                        Info("  -> Slot {}: MatID {}", currentOffset + i, matIdx);
                    }

                    _flatMaterialIndices[currentOffset + i] = matIdx;
                }

                currentOffset += materialCount;
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) processEntity(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) processEntity(e);

            auto processSharedEntity = [&](EntityID entity) {
                if (!overridePool || !overridePool->Has(entity)) return;

                EntityID sharedEntity = overridePool->Get(entity).sharedMaterialEntity;
                if (sharedEntity == NULL_ENTITY) return;

                if (pool->Has(sharedEntity)) {
                    auto& comp = pool->Get(entity);
                    uint32_t masterOffset = pool->Get(sharedEntity).materialOffset;

                    comp.materialOffset = masterOffset;
                    comp.version++;
                    pool->SetBit<CHANGED_BIT>(entity);
                    pool->MarkStaticDirty(entity);
                }
                };

            for (auto e : pool->GetStorage().GetStaticEntities()) processSharedEntity(e);
            for (auto e : pool->GetStorage().GetDynamicEntities()) processSharedEntity(e);
            for (auto e : pool->GetStorage().GetStreamEntities()) processSharedEntity(e);
            
            if (needsRebuild || needsUpload) {
                uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
                this->SetFramesToUpload(framesInFlight);
                scene->GetSceneDrawData()->RequestGlobalSync(framesInFlight);

                if constexpr (ENABLE_DEBUG_LOGGING) {
                    Info("[MaterialSystem UPDATE] Frame {}: Requested global sync and GPU upload for {} frames.", scene->GetSystemContext().frameIndex, framesInFlight);
                }
            }
            });
    }

    void MaterialSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            bool force = this->ShouldForceUpload();

            if (force) {
                this->DecrementFramesToUpload();
            }

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[MaterialSystem UPLOAD] Frame {}: Upload check. Force: {}, FlatIndicesSize: {}", frameIndex, force, _flatMaterialIndices.size());
            }

            if (!force || _flatMaterialIndices.empty()) 
                return;

            auto drawData = scene->GetSceneDrawData();
            size_t reqFromModels = drawData->Models.requiredMaterialBufferSize;
            size_t actualElements = _flatMaterialIndices.size();
            size_t requiredElements = std::max(actualElements, reqFromModels);

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[MaterialSystem UPLOAD] Frame {}: Resizing GPU buffer. Actual CPU Elements: {}, Req from Models: {}, Final GPU Elements: {}", frameIndex, actualElements, reqFromModels, requiredElements);
            }

            drawData->Models.materialIndexBuffer.UpdateCapacity(frameIndex, requiredElements);

            size_t actualDataSize = _flatMaterialIndices.size() * sizeof(uint32_t);
            drawData->Models.materialIndexBuffer.Write(frameIndex, _flatMaterialIndices.data(), actualDataSize, 0);

            if constexpr (ENABLE_DEBUG_LOGGING) {
                Info("[MaterialSystem UPLOAD] Frame {}: GPU buffer written successfully ({} bytes).", frameIndex, actualDataSize);
            }
            });
    }
}