#include "DirectionLightShadowSystem.h"
#include "DirectionLightSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Component/DirectionLightComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Scene/Scene.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Syn
{
    std::vector<TypeID> DirectionLightShadowSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<DirectionLightSystem>::ID,
            TypeInfo<CameraSystem>::ID
        };
    }

    std::vector<TypeID> DirectionLightShadowSystem::GetWriteDependencies() const
    {
        return { TypeInfo<DirectionLightShadowSystem>::ID };
    }

    void DirectionLightShadowSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();
        auto lightPool = registry->GetPool<DirectionLightComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!shadowPool || !lightPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        const auto& cameraComp = cameraPool->Get(cameraEntity);

        ParallelForEachIf<UPDATE_BIT>(shadowPool, subflow, SystemPhaseNames::Update,
            [shadowPool, lightPool, cameraComp](EntityID entity) {
                if (lightPool->Has(entity))
                {
                    auto& shadowComp = shadowPool->Get(entity);
                    auto& lightComp = lightPool->Get(entity);

                    float aspect = cameraComp.width / cameraComp.height;
                    float fovRad = glm::radians(cameraComp.fov);
                    float camNear = cameraComp.nearPlane;
                    float camFar = shadowComp.shadowFarPlane;

                    float splits[5] = { 0.0f, shadowComp.cascadeSplits.x, shadowComp.cascadeSplits.y, shadowComp.cascadeSplits.z, shadowComp.cascadeSplits.w };

                    glm::vec3 camPos = cameraComp.position;
                    glm::vec3 camDir = cameraComp.direction;
                    glm::vec3 camRight = cameraComp.right;
                    glm::vec3 camUp = cameraComp.up;

                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                    if (std::abs(glm::dot(up, lightComp.direction)) > 0.99f) {
                        up = glm::vec3(0.0f, 0.0f, 1.0f);
                    }

                    for (int i = 0; i < 4; ++i)
                    {
                        float sliceNear = camNear + splits[i] * (camFar - camNear);
                        float sliceFar = camNear + splits[i + 1] * (camFar - camNear);

                        float halfFovTan = std::tan(fovRad * 0.5f);
                        float nearHeight = halfFovTan * sliceNear;
                        float nearWidth = nearHeight * aspect;

                        float farHeight = halfFovTan * sliceFar;
                        float farWidth = farHeight * aspect;

                        glm::vec3 centerNear = camPos + camDir * sliceNear;
                        glm::vec3 centerFar = camPos + camDir * sliceFar;

                        std::array<glm::vec3, 8> corners = {
                            centerNear - camUp * nearHeight - camRight * nearWidth,
                            centerNear + camUp * nearHeight - camRight * nearWidth,
                            centerNear + camUp * nearHeight + camRight * nearWidth,
                            centerNear - camUp * nearHeight + camRight * nearWidth,
                            centerFar - camUp * farHeight - camRight * farWidth,
                            centerFar + camUp * farHeight - camRight * farWidth,
                            centerFar + camUp * farHeight + camRight * farWidth,
                            centerFar - camUp * farHeight + camRight * farWidth
                        };

                        glm::vec3 center(0.0f);
                        for (int j = 0; j < 8; ++j) {
                            center += corners[j];
                        }
                        center /= 8.0f;

                        float radius = 0.0f;
                        for (int j = 0; j < 8; ++j) {
                            radius = std::max(radius, glm::distance(center, corners[j]));
                        }

                        glm::mat4 lightView = glm::lookAt(center - lightComp.direction * radius, center, up);

                        glm::vec3 minOrtho(std::numeric_limits<float>::max());
                        glm::vec3 maxOrtho(std::numeric_limits<float>::lowest());

                        for (int j = 0; j < 8; ++j) {
                            glm::vec3 trf = glm::vec3(lightView * glm::vec4(corners[j], 1.0f));
                            minOrtho = glm::min(minOrtho, trf);
                            maxOrtho = glm::max(maxOrtho, trf);
                        }

                        float zMult = 10.0f;
                        if (minOrtho.z < 0) minOrtho.z *= zMult;
                        else minOrtho.z /= zMult;
                        if (maxOrtho.z < 0) maxOrtho.z /= zMult;
                        else maxOrtho.z *= zMult;

                        glm::mat4 orthoProj = glm::ortho(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, minOrtho.z, maxOrtho.z);
                        glm::mat4 viewProj = orthoProj * lightView;
                        shadowComp.cascadeViews[i] = lightView;
                        shadowComp.cascadeProjs[i] = orthoProj;
                        shadowComp.cascadeViewProjs[i] = viewProj;
                        shadowComp.cascadeFrustums[i].Update(viewProj);

                        glm::mat4 orthoProjVulkan = orthoProj;
                        orthoProjVulkan[1][1] *= -1;
                        shadowComp.cascadeViewProjsVulkan[i] = orthoProjVulkan * lightView;
                    }

                    if (shadowPool->IsDynamic(entity))
                        shadowPool->SetBit<CHANGED_BIT>(entity);

                    shadowComp.version++;
                }
            });
    }

    void DirectionLightShadowSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto bufferManager = scene->GetComponentBufferManager();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();
        if (!shadowPool) return;

        auto dataBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightShadowData, frameIndex);
        auto colliderBufferView = bufferManager->GetComponentBuffer(BufferNames::DirectionLightShadowColliderData, frameIndex);

        if (!dataBufferView.buffer || !colliderBufferView.buffer) return;

        auto dataHandler = static_cast<DirectionLightShadowGPU*>(dataBufferView.buffer->Map());
        auto colliderHandler = static_cast<DirectionLightShadowColliderGPU*>(colliderBufferView.buffer->Map());

        auto processUpload = [shadowPool, dataHandler, colliderHandler, dataBufferView](EntityID entity) mutable {
            auto& comp = shadowPool->Get(entity);
            auto denseIndex = shadowPool->GetMapping().Get(entity);

            if (dataBufferView.versions[denseIndex] != comp.version)
            {
                dataBufferView.versions[denseIndex] = comp.version;

                dataHandler[denseIndex] = DirectionLightShadowGPU(comp);
                colliderHandler[denseIndex] = DirectionLightShadowColliderGPU(comp, entity);
            }
            };

        ForEachStream(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(shadowPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}