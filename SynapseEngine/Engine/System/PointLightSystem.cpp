#include "PointLightSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/PointLightShadowComponent.h"
#include "Engine/Scene/Scene.h"

namespace Syn
{
    std::vector<TypeID> PointLightSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    std::vector<TypeID> PointLightSystem::GetWriteDependencies() const
    {
        return { TypeInfo<PointLightSystem>::ID };
    }

    void PointLightSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pointLightPool = registry->GetPool<PointLightComponent>();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();

        if (!pointLightPool || !transformPool) return;

        auto structuralTask = this->EmplaceTask(subflow, "StructuralToggle", [registry, pointLightPool, shadowPool]() {
            auto processToggle = [&](const std::span<const EntityID>& entities) {
                for (EntityID entity : entities)
                {
                    if (pointLightPool->IsBitSet<SHADOW_TOGGLE_BIT>(entity))
                    {
                        auto& lightComp = pointLightPool->Get(entity);
                        bool hasShadow = shadowPool ? shadowPool->Has(entity) : false;

                        if (lightComp.useShadow && !hasShadow) {
                            registry->AddComponents<PointLightShadowComponent>(entity);

                            if (pointLightPool->IsStatic(entity)) {
                                shadowPool->SetCategory(entity, StorageCategory::Static);
                                shadowPool->MarkStaticDirty(entity);
                            }
                            else if (pointLightPool->IsStream(entity)) {
                                shadowPool->SetCategory(entity, StorageCategory::Stream);
                            }
                            else {
                                shadowPool->SetCategory(entity, StorageCategory::Dynamic);
                            }
                        }
                        else if (!lightComp.useShadow && hasShadow) {
                            registry->RemoveComponents<PointLightShadowComponent>(entity);
                        }
                    }
                }
                };

            processToggle(pointLightPool->GetStorage().GetStreamEntities());
            processToggle(pointLightPool->GetStorage().GetDynamicEntities());
            processToggle(pointLightPool->GetStorage().GetStaticEntities());
            });

        auto mathTasks = ParallelForEachIf<UPDATE_BIT>(pointLightPool, subflow, SystemPhaseNames::Update,
            [pointLightPool, transformPool, shadowPool](EntityID entity) {
                if (transformPool->Has(entity) && transformPool->IsBitSet<TRANSFORM_POS_CHANGED>(entity))
                {
                    auto& transformComp = transformPool->Get(entity);
                    auto& lightComp = pointLightPool->Get(entity);

                    lightComp.position = glm::vec3(transformComp.transform[3]);

                    //??
                    if (pointLightPool->IsDynamic(entity))
                        pointLightPool->SetBit<CHANGED_BIT>(entity);

                    lightComp.version++;

                    // ??
                    if (shadowPool && shadowPool->Has(entity)) {
                        shadowPool->SetBit<CHANGED_BIT>(entity);
                    }
                }
            });

        for (auto& task : mathTasks) {
            structuralTask.precede(task);
        }
    }

    void PointLightSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto pointLightPool = registry->GetPool<PointLightComponent>();
        if (!pointLightPool) return;

        auto dataBufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightData, frameIndex);
        auto colliderBufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightColliderData, frameIndex);

        if (!dataBufferView.buffer || !colliderBufferView.buffer) return;

        auto dataHandler = static_cast<PointLightComponentGPU*>(dataBufferView.buffer->Map());
        auto colliderHandler = static_cast<PointLightColliderGPU*>(colliderBufferView.buffer->Map());

        auto processUpload = [pointLightPool, dataHandler, colliderHandler, dataBufferView](EntityID entity) mutable {
            auto& comp = pointLightPool->Get(entity);
            auto denseIndex = pointLightPool->GetMapping().Get(entity);

            if (dataBufferView.versions[denseIndex] != comp.version)
            {
                dataBufferView.versions[denseIndex] = comp.version;
                dataHandler[denseIndex] = PointLightComponentGPU(comp);
                colliderHandler[denseIndex] = PointLightColliderGPU(comp, entity);
            }
            };

        ForEachStream(pointLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(pointLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(pointLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}