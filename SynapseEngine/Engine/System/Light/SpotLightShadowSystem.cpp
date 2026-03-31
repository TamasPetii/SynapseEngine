#include "SpotLightShadowSystem.h"
#include "SpotLightSystem.h"
#include "Engine/Component/SpotLightComponent.h"
#include "Engine/Scene/Scene.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    std::vector<TypeID> SpotLightShadowSystem::GetReadDependencies() const
    {
        return { TypeInfo<SpotLightSystem>::ID };
    }

    std::vector<TypeID> SpotLightShadowSystem::GetWriteDependencies() const
    {
        return { TypeInfo<SpotLightShadowSystem>::ID };
    }

    void SpotLightShadowSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        auto lightPool = registry->GetPool<SpotLightComponent>();

        if (!shadowPool || !lightPool) return;

        ParallelForEachIf<UPDATE_BIT>(shadowPool, subflow, SystemPhaseNames::Update,
            [shadowPool, lightPool](EntityID entity) {
                if (lightPool->Has(entity))
                {
                    auto& shadowComp = shadowPool->Get(entity);
                    auto& lightComp = lightPool->Get(entity);

                    shadowComp.farPlane = lightComp.range;

                    float fovY = glm::radians(lightComp.outerAngle * 2.0f);
                    glm::mat4 shadowProj = glm::perspective(fovY, 1.0f, shadowComp.nearPlane, shadowComp.farPlane);
                    shadowProj[1][1] *= -1;

                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                    if (std::abs(glm::dot(up, lightComp.direction)) > 0.99f) {
                        up = glm::vec3(0.0f, 0.0f, 1.0f);
                    }

                    shadowComp.viewProj = shadowProj * glm::lookAt(lightComp.position, lightComp.position + lightComp.direction, up);

                    if (shadowPool->IsDynamic(entity))
                        shadowPool->SetBit<CHANGED_BIT>(entity);

                    shadowComp.version++;
                }
            });
    }

    void SpotLightShadowSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        if (!shadowPool) return;

        auto bufferView = bufferManager->GetComponentBuffer(BufferNames::SpotLightShadowData, frameIndex);
        if (!bufferView.buffer) return;

        auto bufferHandler = static_cast<SpotLightShadowComponentGPU*>(bufferView.buffer->Map());

        auto processUpload = [shadowPool, bufferHandler, bufferView](EntityID entity) mutable {
            auto& comp = shadowPool->Get(entity);
            auto denseIndex = shadowPool->GetMapping().Get(entity);

            if (bufferView.versions[denseIndex] != comp.version)
            {
                bufferView.versions[denseIndex] = comp.version;
                bufferHandler[denseIndex] = SpotLightShadowComponentGPU(comp);
            }
            };

        ForEachStream(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}