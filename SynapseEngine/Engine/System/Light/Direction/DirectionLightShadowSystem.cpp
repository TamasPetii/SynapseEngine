// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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

                        // Calculate actual camera frustum center and radius
                        glm::vec3 frustumCenter(0.0f);
                        for (int j = 0; j < 8; ++j) {
                            frustumCenter += corners[j];
                        }
                        frustumCenter /= 8.0f;

                        float frustumRadius = 0.0f;
                        for (int j = 0; j < 8; ++j) {
                            frustumRadius = std::max(frustumRadius, glm::distance(frustumCenter, corners[j]));
                        }

                        // Round radius to prevent float precision flickering on micro-rotations
                        frustumRadius = std::ceil(frustumRadius * 16.0f) / 16.0f;

                        // Padded Bounding Sphere (20% padding)
                        float paddedRadius = frustumRadius * 1.20f;
                        float threshold = paddedRadius - frustumRadius;

                        // Check anchor distance
                        float dist = glm::distance(frustumCenter, shadowComp.cascadeAnchors[i]);

                        // Threshold crossed or first initialization (Invalidation)
                        if (dist > threshold || shadowComp.cascadeRadius[i] == 0.0f)
                        {
                            // Extract dynamic resolution from Atlas Rect UV coordinates
                            float uvWidth = shadowComp.cascadeAtlasRects[i].z;
                            float cascadeResolution = uvWidth > 0.0f ? (uvWidth * SHADOW_ATLAS_SIZE) : 2048.0f;
                            float worldUnitsPerTexel = (paddedRadius * 2.0f) / cascadeResolution;

                            // Map center to light space for texel snapping
                            glm::mat4 lightSpace = glm::lookAt(glm::vec3(0.0f), lightComp.direction, up);
                            glm::vec3 lightSpaceCenter = glm::vec3(lightSpace * glm::vec4(frustumCenter, 1.0f));

                            // Snap to texel size to avoid edge shimmering
                            lightSpaceCenter.x = std::floor(lightSpaceCenter.x / worldUnitsPerTexel) * worldUnitsPerTexel;
                            lightSpaceCenter.y = std::floor(lightSpaceCenter.y / worldUnitsPerTexel) * worldUnitsPerTexel;

                            // Transform back to world space for the fixed anchor
                            glm::mat4 invLightSpace = glm::inverse(lightSpace);
                            glm::vec3 snappedCenter = glm::vec3(invLightSpace * glm::vec4(lightSpaceCenter, 1.0f));

                            shadowComp.cascadeAnchors[i] = snappedCenter;
                            shadowComp.cascadeRadius[i] = paddedRadius;
                            shadowComp.isStaticDirty[i] = true;
                        }
                        else
                        {
                            shadowComp.isStaticDirty[i] = false;
                        }

                        // Generate matrices based on the fixed anchor
                        glm::vec3 cascadeAnchorCenter = shadowComp.cascadeAnchors[i];
                        float cascadeAnchorRadius = shadowComp.cascadeRadius[i];

                        // Light view matrix looking at the sphere anchor center
                        glm::mat4 lightView = glm::lookAt(cascadeAnchorCenter - lightComp.direction * cascadeAnchorRadius, cascadeAnchorCenter, up);

                        // Fixed orthographic extents based on padded radius
                        glm::vec3 minOrtho(-cascadeAnchorRadius, -cascadeAnchorRadius, 0.0f);
                        glm::vec3 maxOrtho(cascadeAnchorRadius, cascadeAnchorRadius, 0.0f);

                        // Fix Z-bounds to the anchor to maintain static shadow depth buffer integrity
                        float zNear = -cascadeAnchorRadius - 1000.0f;
                        float zFar = cascadeAnchorRadius + 500.0f;

                        minOrtho.z = -zFar;
                        maxOrtho.z = -zNear;

                        shadowComp.cascadeAabbMin[i] = minOrtho;
                        shadowComp.cascadeAabbMax[i] = maxOrtho;

                        // Create projection and view-projection matrices
                        glm::mat4 orthoProj = glm::orthoZO(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, zNear, zFar);
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
                            Info("    Anchor Center: ({:.2f}, {:.2f}, {:.2f}) | Padded Radius: {:.2f}", cascadeAnchorCenter.x, cascadeAnchorCenter.y, cascadeAnchorCenter.z, cascadeAnchorRadius);
                            Info("    Is Static Dirty: {}", shadowComp.isStaticDirty[i]);
                            Info("    OrthoMin: ({:.2f}, {:.2f}, {:.2f})", minOrtho.x, minOrtho.y, minOrtho.z);
                            Info("    OrthoMax: ({:.2f}, {:.2f}, {:.2f})", maxOrtho.x, maxOrtho.y, maxOrtho.z);
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