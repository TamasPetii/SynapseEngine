#include "DirectionLightSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/DirectionLightShadowComponent.h"
#include "Engine/Scene/Scene.h"

namespace Syn
{
    std::vector<TypeID> DirectionLightSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    std::vector<TypeID> DirectionLightSystem::GetWriteDependencies() const
    {
        return { TypeInfo<DirectionLightSystem>::ID };
    }

    void DirectionLightSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto lightPool = registry->GetPool<DirectionLightComponent>();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();

        if (!lightPool || !transformPool) return;

        auto structuralTask = this->EmplaceTask(subflow, "StructuralToggle", [registry, lightPool, shadowPool]() {
            auto processToggle = [&](const std::span<const EntityID>& entities) {
                for (EntityID entity : entities)
                {
                    if (lightPool->IsBitSet<SHADOW_TOGGLE_BIT>(entity))
                    {
                        auto& lightComp = lightPool->Get(entity);
                        bool hasShadow = shadowPool ? shadowPool->Has(entity) : false;

                        if (lightComp.useShadow && !hasShadow) {
                            registry->AddComponent<DirectionLightShadowComponent>(entity);
                            auto currentShadowPool = registry->GetPool<DirectionLightShadowComponent>();

                            if (lightPool->IsStatic(entity)) {
                                currentShadowPool->SetCategory(entity, StorageCategory::Static);
                                currentShadowPool->MarkStaticDirty(entity);
                            }
                            else if (lightPool->IsDynamic(entity)) {
                                currentShadowPool->SetCategory(entity, StorageCategory::Dynamic);
                            }
                            else {
                                currentShadowPool->SetCategory(entity, StorageCategory::Stream);
                            }
                        }
                        else if (!lightComp.useShadow && hasShadow) {
                            registry->RemoveComponents<DirectionLightShadowComponent>(entity);
                        }
                    }
                }
                };

            processToggle(lightPool->GetStorage().GetStreamEntities());
            processToggle(lightPool->GetStorage().GetDynamicEntities());
            processToggle(lightPool->GetStorage().GetStaticEntities());
            });

        auto mathTasks = ParallelForEachIf<UPDATE_BIT>(lightPool, subflow, SystemPhaseNames::Update,
            [lightPool, transformPool, registry](EntityID entity) {
                if (transformPool->Has(entity) && transformPool->IsBitSet<TRANSFORM_ROT_CHANGED>(entity))
                {
                    auto& transformComp = transformPool->Get(entity);
                    auto& lightComp = lightPool->Get(entity);

                    lightComp.direction = glm::normalize(glm::vec3(-transformComp.transform[2]));

                    if (lightPool->IsDynamic(entity))
                        lightPool->SetBit<CHANGED_BIT>(entity);

                    lightComp.version++;

                    auto currentShadowPool = registry->GetPool<DirectionLightShadowComponent>();
                    if (currentShadowPool && currentShadowPool->Has(entity)) {
                        if (currentShadowPool->IsStatic(entity))
                            currentShadowPool->MarkStaticDirty(entity);
                        else if (currentShadowPool->IsDynamic(entity))
                            currentShadowPool->SetBit<UPDATE_BIT>(entity);
                    }
                }
            });

        for (auto& task : mathTasks) {
            structuralTask.precede(task);
        }
    }

    void DirectionLightSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto lightPool = registry->GetPool<DirectionLightComponent>();
        if (!lightPool) return;

        auto dataBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightData, frameIndex);
        if (!dataBufferView.buffer) return;

        auto dataHandler = static_cast<DirectionLightGPU*>(dataBufferView.buffer->Map());

        auto processUpload = [lightPool, dataHandler, dataBufferView](EntityID entity) mutable {
            auto& comp = lightPool->Get(entity);
            auto denseIndex = lightPool->GetMapping().Get(entity);

            if (dataBufferView.versions[denseIndex] != comp.version)
            {
                dataBufferView.versions[denseIndex] = comp.version;
                dataHandler[denseIndex] = DirectionLightGPU(comp);
            }
            };

        ForEachStream(lightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(lightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(lightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}