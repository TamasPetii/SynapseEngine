#include "SpotLightSystem.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/SpotLightShadowComponent.h"
#include "Engine/Scene/Scene.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    std::vector<TypeID> SpotLightSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    std::vector<TypeID> SpotLightSystem::GetWriteDependencies() const
    {
        return { TypeInfo<SpotLightSystem>::ID };
    }

    void SpotLightSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto spotLightPool = registry->GetPool<SpotLightComponent>();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();

        if (!spotLightPool || !transformPool) return;

        auto structuralTask = this->EmplaceTask(subflow, "StructuralToggle", [registry, spotLightPool, shadowPool]() {
            auto processToggle = [&](const std::span<const EntityID>& entities) {
                for (EntityID entity : entities)
                {
                    if (spotLightPool->IsBitSet<SHADOW_TOGGLE_BIT>(entity))
                    {
                        auto& lightComp = spotLightPool->Get(entity);
                        bool hasShadow = shadowPool ? shadowPool->Has(entity) : false;

                        if (lightComp.useShadow && !hasShadow) {
                            registry->AddComponent<SpotLightShadowComponent>(entity);
                            auto currentShadowPool = registry->GetPool<SpotLightShadowComponent>();

                            if (spotLightPool->IsStatic(entity)) {
                                currentShadowPool->SetCategory(entity, StorageCategory::Static);
                                currentShadowPool->MarkStaticDirty(entity);
                            }
                            else if (spotLightPool->IsDynamic(entity)) {
                                currentShadowPool->SetCategory(entity, StorageCategory::Dynamic);
                            }
                            else {
                                currentShadowPool->SetCategory(entity, StorageCategory::Stream);
                            }
                        }
                        else if (!lightComp.useShadow && hasShadow) {
                            registry->RemoveComponents<SpotLightShadowComponent>(entity);
                        }
                    }
                }
                };

            processToggle(spotLightPool->GetStorage().GetStreamEntities());
            processToggle(spotLightPool->GetStorage().GetDynamicEntities());
            processToggle(spotLightPool->GetStorage().GetStaticEntities());
            });

        auto mathTasks = ParallelForEachIf<UPDATE_BIT>(spotLightPool, subflow, SystemPhaseNames::Update,
            [spotLightPool, transformPool, registry](EntityID entity) {
                if (transformPool->Has(entity) && (transformPool->IsBitSet<TRANSFORM_POS_CHANGED>(entity) || transformPool->IsBitSet<TRANSFORM_ROT_CHANGED>(entity)))
                {
                    auto& transformComp = transformPool->Get(entity);
                    auto& lightComp = spotLightPool->Get(entity);

                    lightComp.position = glm::vec3(transformComp.transform[3]);
                    lightComp.direction = glm::normalize(glm::vec3(-transformComp.transform[2]));

                    float outerRad = glm::radians(lightComp.outerAngle);
                    float cosOuter = std::cos(outerRad);
                    float sinOuter = std::sin(outerRad);

                    if (lightComp.outerAngle < 45.0f) {
                        lightComp.sphereCollider.radius = lightComp.range / (2.0f * cosOuter * cosOuter);
                        lightComp.sphereCollider.center = lightComp.position + lightComp.direction * lightComp.sphereCollider.radius;
                    }
                    else {
                        lightComp.sphereCollider.radius = lightComp.range * sinOuter;
                        lightComp.sphereCollider.center = lightComp.position + lightComp.direction * (lightComp.range * cosOuter);
                    }

                    glm::vec3 baseCenter = lightComp.position + lightComp.direction * lightComp.range;
                    float baseRadius = lightComp.range * std::tan(outerRad);

                    glm::vec3 diskExtents(
                        baseRadius * std::sqrt(std::max(0.0f, 1.0f - lightComp.direction.x * lightComp.direction.x)),
                        baseRadius * std::sqrt(std::max(0.0f, 1.0f - lightComp.direction.y * lightComp.direction.y)),
                        baseRadius * std::sqrt(std::max(0.0f, 1.0f - lightComp.direction.z * lightComp.direction.z))
                    );

                    glm::vec3 baseMin = baseCenter - diskExtents;
                    glm::vec3 baseMax = baseCenter + diskExtents;

                    lightComp.aabbCollider.min = glm::min(lightComp.position, baseMin);
                    lightComp.aabbCollider.max = glm::max(lightComp.position, baseMax);

                    glm::vec3 new_Y = -lightComp.direction;
                    glm::vec3 world_up = (std::abs(new_Y.y) < 0.999f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                    glm::vec3 new_X = glm::normalize(glm::cross(world_up, new_Y));
                    glm::vec3 new_Z = glm::normalize(glm::cross(new_X, new_Y));

                    glm::mat4 rot(1.0f);
                    rot[0] = glm::vec4(new_X, 0.0f);
                    rot[1] = glm::vec4(new_Y, 0.0f);
                    rot[2] = glm::vec4(new_Z, 0.0f);

                    float enclosingRadius = baseRadius * glm::sqrt(2.f);
                    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(enclosingRadius, lightComp.range, enclosingRadius));
                    glm::vec3 centerPos = lightComp.position + lightComp.direction * (lightComp.range * 0.5f);
                    glm::mat4 trans = glm::translate(glm::mat4(1.0f), centerPos);

                    lightComp.transform = trans * rot * scale;

                    if (spotLightPool->IsDynamic(entity))
                        spotLightPool->SetBit<CHANGED_BIT>(entity);

                    lightComp.version++;

                    auto currentShadowPool = registry->GetPool<SpotLightShadowComponent>();
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

    void SpotLightSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto spotLightPool = registry->GetPool<SpotLightComponent>();
        if (!spotLightPool) return;

        auto dataBufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightData, frameIndex);
        auto colliderBufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightColliderData, frameIndex);

        if (!dataBufferView.buffer || !colliderBufferView.buffer) return;

        auto dataHandler = static_cast<SpotLightComponentGPU*>(dataBufferView.buffer->Map());
        auto colliderHandler = static_cast<SpotLightColliderGPU*>(colliderBufferView.buffer->Map());

        auto processUpload = [spotLightPool, dataHandler, colliderHandler, dataBufferView](EntityID entity) mutable {
            auto& comp = spotLightPool->Get(entity);
            auto denseIndex = spotLightPool->GetMapping().Get(entity);

            if (dataBufferView.versions[denseIndex] != comp.version)
            {
                dataBufferView.versions[denseIndex] = comp.version;
                dataHandler[denseIndex] = SpotLightComponentGPU(comp);
                colliderHandler[denseIndex] = SpotLightColliderGPU(comp, entity);
            }
            };

        ForEachStream(spotLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(spotLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(spotLightPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}