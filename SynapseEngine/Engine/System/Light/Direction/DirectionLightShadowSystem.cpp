#include "DirectionLightShadowSystem.h"
#include "DirectionLightSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Scene/Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Logger/SynLog.h"
#include "DirectionLightShadowAtlasSystem.h"

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> DirectionLightShadowSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<DirectionLightSystem>::ID,
            TypeInfo<DirectionLightShadowAtlasSystem>::ID,
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

                    // Camera properties
                    float aspect = cameraComp.width / cameraComp.height;
                    float fovRad = glm::radians(cameraComp.fov);
                    float camNear = cameraComp.nearPlane;
                    float camFar = shadowComp.shadowFarPlane;

                    float splits[5] = { 0.0f, shadowComp.cascadeSplits.x, shadowComp.cascadeSplits.y, shadowComp.cascadeSplits.z, shadowComp.cascadeSplits.w };

                    glm::vec3 camPos = cameraComp.position;
                    glm::vec3 camDir = cameraComp.direction;
                    glm::vec3 camRight = cameraComp.right;
                    glm::vec3 camUp = cameraComp.up;

                    // Prevent collinearity with light direction
                    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                    if (std::abs(glm::dot(up, lightComp.direction)) > 0.99f) {
                        up = glm::vec3(0.0f, 0.0f, 1.0f);
                    }

                    if constexpr (ENABLE_DEBUG_LOGGING) {
                        Info("--- SHADOW CASCADE UPDATE ---");
                        Info("Camera -> FOV: {}, Aspect: {}, Near: {}, ShadowFar: {}", cameraComp.fov, aspect, camNear, camFar);
                    }

                    for (int i = 0; i < 4; ++i)
                    {
                        // Calculate split slice distances
                        float sliceNear = camNear + splits[i] * (camFar - camNear);
                        float sliceFar = camNear + splits[i + 1] * (camFar - camNear);

                        // Frustum slice dimensions
                        float halfFovTan = std::tan(fovRad * 0.5f);
                        float nearHeight = halfFovTan * sliceNear;
                        float nearWidth = nearHeight * aspect;

                        float farHeight = halfFovTan * sliceFar;
                        float farWidth = farHeight * aspect;

                        // Center points of near and far planes
                        glm::vec3 centerNear = camPos + camDir * sliceNear;
                        glm::vec3 centerFar = camPos + camDir * sliceFar;

                        // 8 corners of the frustum sub-slice
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

                        // Calculate bounding sphere center and radius
                        glm::vec3 center(0.0f);
                        for (int j = 0; j < 8; ++j) {
                            center += corners[j];
                        }
                        center /= 8.0f;

                        float radius = 0.0f;
                        for (int j = 0; j < 8; ++j) {
                            radius = std::max(radius, glm::distance(center, corners[j]));
                        }

                        // Light view matrix looking at the sphere center
                        glm::mat4 lightView = glm::lookAt(center - lightComp.direction * radius, center, up);

                        // Calculate Orthographic AABB in light space
                        glm::vec3 minOrtho(std::numeric_limits<float>::max());
                        glm::vec3 maxOrtho(std::numeric_limits<float>::lowest());

                        // Expand Z bounds to capture objects behind the camera
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

                        shadowComp.cascadeAabbMin[i] = minOrtho;
                        shadowComp.cascadeAabbMax[i] = maxOrtho;

                        // Create projection and view-projection matrices
                        glm::mat4 orthoProj = glm::ortho(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, minOrtho.z, maxOrtho.z);
                        glm::mat4 viewProj = orthoProj * lightView;
                        shadowComp.cascadeViews[i] = lightView;
                        shadowComp.cascadeProjs[i] = orthoProj;
                        shadowComp.cascadeViewProjs[i] = viewProj;

                        // Update frustum collider for culling
                        glm::mat4 viewT = glm::transpose(lightView);

                        glm::vec4 leftPlane = viewT * glm::vec4(1.0f, 0.0f, 0.0f, -minOrtho.x);
                        glm::vec4 rightPlane = viewT * glm::vec4(-1.0f, 0.0f, 0.0f, maxOrtho.x);
                        glm::vec4 bottomPlane = viewT * glm::vec4(0.0f, 1.0f, 0.0f, -minOrtho.y);
                        glm::vec4 topPlane = viewT * glm::vec4(0.0f, -1.0f, 0.0f, maxOrtho.y);
                        glm::vec4 zMinPlane = viewT * glm::vec4(0.0f, 0.0f, 1.0f, -minOrtho.z);
                        glm::vec4 zMaxPlane = viewT * glm::vec4(0.0f, 0.0f, -1.0f, maxOrtho.z);

                        shadowComp.cascadeFrustums[i].planes[0] = FrustumCollider::NormalizePlane(zMinPlane);
                        shadowComp.cascadeFrustums[i].planes[1] = FrustumCollider::NormalizePlane(rightPlane);
                        shadowComp.cascadeFrustums[i].planes[2] = FrustumCollider::NormalizePlane(leftPlane);
                        shadowComp.cascadeFrustums[i].planes[3] = FrustumCollider::NormalizePlane(topPlane);
                        shadowComp.cascadeFrustums[i].planes[4] = FrustumCollider::NormalizePlane(bottomPlane);
                        shadowComp.cascadeFrustums[i].planes[5] = FrustumCollider::NormalizePlane(zMaxPlane);

                        glm::mat4 orthoProjVulkan = orthoProj;
                        orthoProjVulkan[1][1] *= -1;
                        shadowComp.cascadeViewProjsVulkan[i] = orthoProjVulkan * lightView;

                        if constexpr (ENABLE_DEBUG_LOGGING) {
                            Info("  Cascade {}: Splits [{} - {}]", i, splits[i], splits[i + 1]);
                            Info("    Center: ({:.2f}, {:.2f}, {:.2f}) | Radius: {:.2f}", center.x, center.y, center.z, radius);
                            Info("    OrthoMin: ({:.2f}, {:.2f}, {:.2f})", minOrtho.x, minOrtho.y, minOrtho.z);
                            Info("    OrthoMax: ({:.2f}, {:.2f}, {:.2f})", maxOrtho.x, maxOrtho.y, maxOrtho.z);

                            const auto& planes = shadowComp.cascadeFrustums[i].planes;
                            Info("    Frustum Planes (nx, ny, nz, d):");
                            Info("      Near:   ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[0].x, planes[0].y, planes[0].z, planes[0].w);
                            Info("      Right:  ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[1].x, planes[1].y, planes[1].z, planes[1].w);
                            Info("      Left:   ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[2].x, planes[2].y, planes[2].z, planes[2].w);
                            Info("      Top:    ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[3].x, planes[3].y, planes[3].z, planes[3].w);
                            Info("      Bottom: ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[4].x, planes[4].y, planes[4].z, planes[4].w);
                            Info("      Far:    ({:.2f}, {:.2f}, {:.2f}, {:.2f})", planes[5].x, planes[5].y, planes[5].z, planes[5].w);
                        }
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

            // Only upload if component version changed
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