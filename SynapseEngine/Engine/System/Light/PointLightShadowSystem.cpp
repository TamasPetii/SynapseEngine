#include "PointLightShadowSystem.h"
#include "PointLightSystem.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Scene/Scene.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    std::vector<TypeID> PointLightShadowSystem::GetReadDependencies() const
    {
        return { TypeInfo<PointLightSystem>::ID };
    }

    std::vector<TypeID> PointLightShadowSystem::GetWriteDependencies() const
    {
        return { TypeInfo<PointLightShadowSystem>::ID };
    }

    void PointLightShadowSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        auto lightPool = registry->GetPool<PointLightComponent>();

        if (!shadowPool || !lightPool) return;

        static const glm::vec3 directions[6] = {
            { 1.0,  0.0,  0.0 }, {-1.0,  0.0,  0.0 },
            { 0.0,  1.0,  0.0 }, { 0.0, -1.0,  0.0 },
            { 0.0,  0.0,  1.0 }, { 0.0,  0.0, -1.0 }
        };

        static const glm::vec3 upVectors[6] = {
            { 0.0, -1.0,  0.0 }, { 0.0, -1.0,  0.0 },
            { 0.0,  0.0,  1.0 }, { 0.0,  0.0, -1.0 },
            { 0.0, -1.0,  0.0 }, { 0.0, -1.0,  0.0 }
        };

        ParallelForEachIf<UPDATE_BIT>(shadowPool, subflow, SystemPhaseNames::Update,
            [shadowPool, lightPool](EntityID entity) {
                if (lightPool->Has(entity))
                {
                    auto& shadowComp = shadowPool->Get(entity);
                    auto& lightComp = lightPool->Get(entity);

                    //Todo: Dynamic Distance Based Doom Rects

                    shadowComp.farPlane = lightComp.radius;

                    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), 1.0f, shadowComp.nearPlane, shadowComp.farPlane);
                    shadowProj[1][1] *= -1;

                    for (int i = 0; i < 6; ++i) {
                        shadowComp.viewProjs[i] = shadowProj * glm::lookAt(lightComp.position, lightComp.position + directions[i], upVectors[i]);
                    }

                    if(shadowPool->IsDynamic(entity))
						shadowPool->SetBit<CHANGED_BIT>(entity);

                    shadowComp.version++;
                }
            });
    }

    void PointLightShadowSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        if (!shadowPool) return;

        auto bufferView = bufferManager->GetComponentBuffer(BufferNames::PointLightShadowData, frameIndex);
        if (!bufferView.buffer) return;

        auto bufferHandler = static_cast<PointLightShadowComponentGPU*>(bufferView.buffer->Map());

        auto processUpload = [shadowPool, bufferHandler, bufferView](EntityID entity) mutable {
            auto& comp = shadowPool->Get(entity);
            auto denseIndex = shadowPool->GetMapping().Get(entity);

            if (bufferView.versions[denseIndex] != comp.version)
            {
                bufferView.versions[denseIndex] = comp.version;
                bufferHandler[denseIndex] = PointLightShadowComponentGPU(comp);
            }
            };

        ForEachStream(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}