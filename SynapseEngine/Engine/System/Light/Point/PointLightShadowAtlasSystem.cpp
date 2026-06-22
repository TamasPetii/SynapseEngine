#include "PointLightShadowAtlasSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Logger/SynLog.h"
#include "PointLightCullingSystem.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <array>
#include <vector>
#include <algorithm>

namespace Syn
{
    constexpr bool ENABLE_ATLAS_DEBUG_LOGGING = false;

    struct PointLightAllocData {
        EntityID entity;
        uint32_t faceBlockSizePx;
        uint32_t faceBlocksRequired;
    };

    std::vector<TypeID> PointLightShadowAtlasSystem::GetReadDependencies() const {
        return {
            TypeInfo<PointLightCullingSystem>::ID
        };
    }

    std::vector<TypeID> PointLightShadowAtlasSystem::GetWriteDependencies() const {
        return {
            TypeInfo<PointLightShadowAtlasSystem>::ID
        };
    }

    void PointLightShadowAtlasSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto lightPool = registry->GetPool<PointLightComponent>();
        auto shadowPool = registry->GetPool<PointLightShadowComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!shadowPool || !lightPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        this->EmplaceTask(subflow, "Update Point Shadow Atlas", [drawData, lightPool, shadowPool, cameraPool, cameraEntity]() {

            uint32_t activeLights = drawData->PointLightShadow.visibleLightCount;

            if (activeLights == 0)
                return;

            std::fill(drawData->PointLightShadow.gridLookupData.Data(),
                drawData->PointLightShadow.gridLookupData.Data() + (POINT_SHADOW_GRID_SIZE * POINT_SHADOW_GRID_SIZE),
                0xFFFFFFFF);

            const auto& cameraComp = cameraPool->Get(cameraEntity);
            glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

            std::vector<PointLightAllocData> allocRequests;
            allocRequests.reserve(activeLights);

            // Screen space size evaluation
            for (uint32_t i = 0; i < activeLights; ++i)
            {
                EntityID entity = drawData->PointLightShadow.visibleLights[i];
                const auto& lightComp = lightPool->Get(entity);

                float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                    lightComp.position, lightComp.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                // This determines the resolution of ONE face.
                uint32_t faceBlockSizePx = POINT_SHADOW_MIN_BLOCK_SIZE;
                if (screenSizePixels > 1024.0f) faceBlockSizePx = 512;
                else if (screenSizePixels > 512.0f) faceBlockSizePx = 256;
                else if (screenSizePixels > 256.0f) faceBlockSizePx = 128;
                else if (screenSizePixels > 128.0f) faceBlockSizePx = 64;

                allocRequests.push_back({
                    entity,
                    faceBlockSizePx,
                    faceBlockSizePx / POINT_SHADOW_MIN_BLOCK_SIZE
                    });
            }

            // Sort descending to ensure larger blocks are packed first
            std::sort(allocRequests.begin(), allocRequests.end(), [](const PointLightAllocData& a, const PointLightAllocData& b) {
                return a.faceBlockSizePx > b.faceBlockSizePx;
                });

            std::array<std::array<bool, POINT_SHADOW_GRID_SIZE>, POINT_SHADOW_GRID_SIZE> grid = { false };

            // Allocate a 3x2 grid region for the 6 faces
            auto AllocateBlock = [&](uint32_t sizeBlocksX, uint32_t sizeBlocksY, uint32_t& outX, uint32_t& outY) -> bool {
                if (sizeBlocksX > POINT_SHADOW_GRID_SIZE || sizeBlocksY > POINT_SHADOW_GRID_SIZE) return false;

                for (uint32_t y = 0; y <= POINT_SHADOW_GRID_SIZE - sizeBlocksY; y += 1) {
                    for (uint32_t x = 0; x <= POINT_SHADOW_GRID_SIZE - sizeBlocksX; x += 1) {

                        bool isFree = true;
                        for (uint32_t by = 0; by < sizeBlocksY; ++by) {
                            for (uint32_t bx = 0; bx < sizeBlocksX; ++bx) {
                                if (grid[y + by][x + bx]) {
                                    isFree = false;
                                    break;
                                }
                            }
                            if (!isFree) break;
                        }

                        if (isFree) {
                            for (uint32_t by = 0; by < sizeBlocksY; ++by) {
                                for (uint32_t bx = 0; bx < sizeBlocksX; ++bx) {
                                    grid[y + by][x + bx] = true;
                                }
                            }
                            outX = x;
                            outY = y;
                            return true;
                        }
                    }
                }
                return false;
                };

            for (const auto& request : allocRequests)
            {
                auto& shadowComp = shadowPool->Get(request.entity);
                uint32_t gridX = 0, gridY = 0;

                // 3 blocks wide and 2 blocks high to fit all 6 faces
                uint32_t reqBlocksX = request.faceBlocksRequired * 3;
                uint32_t reqBlocksY = request.faceBlocksRequired * 2;

                if (AllocateBlock(reqBlocksX, reqBlocksY, gridX, gridY))
                {
                    float baseUvX = static_cast<float>(gridX) / POINT_SHADOW_GRID_SIZE;
                    float baseUvY = static_cast<float>(gridY) / POINT_SHADOW_GRID_SIZE;
                    float totalUvW = static_cast<float>(reqBlocksX) / POINT_SHADOW_GRID_SIZE;
                    float totalUvH = static_cast<float>(reqBlocksY) / POINT_SHADOW_GRID_SIZE;

                    shadowComp.mainAtlasRect = glm::vec4(baseUvX, baseUvY, totalUvW, totalUvH);

                    float faceUvW = static_cast<float>(request.faceBlocksRequired) / POINT_SHADOW_GRID_SIZE;
                    float faceUvH = static_cast<float>(request.faceBlocksRequired) / POINT_SHADOW_GRID_SIZE;

                    for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
                        uint32_t localFaceX = faceIndex % 3;
                        uint32_t localFaceY = faceIndex / 3;

                        float faceUvX = baseUvX + (localFaceX * faceUvW);
                        float faceUvY = baseUvY + (localFaceY * faceUvH);

                        shadowComp.atlasRects[faceIndex] = glm::vec4(faceUvX, faceUvY, faceUvW, faceUvH);
                    }

                    // Register ownership in the lookup buffer
                    for (uint32_t by = 0; by < reqBlocksY; ++by) {
                        for (uint32_t bx = 0; bx < reqBlocksX; ++bx) {
                            uint32_t flatIndex = (gridY + by) * POINT_SHADOW_GRID_SIZE + (gridX + bx);
                            drawData->PointLightShadow.gridLookupData[flatIndex] = static_cast<uint32_t>(request.entity);
                        }
                    }

                    if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                        Info("PointLight Atlas Alloc - Entity: {} -> X: {}, Y: {}, FaceSize: {}x{}",
                            static_cast<uint32_t>(request.entity),
                            gridX * POINT_SHADOW_MIN_BLOCK_SIZE,
                            gridY * POINT_SHADOW_MIN_BLOCK_SIZE,
                            request.faceBlockSizePx, request.faceBlockSizePx);
                    }
                }
                else
                {
                    shadowComp.mainAtlasRect = glm::vec4(0.0f);
                    shadowComp.atlasRects.fill(glm::vec4(0.0f));

                    if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                        Warning("PointLight Atlas Full! Dropped shadow for entity {}.", static_cast<uint32_t>(request.entity));
                    }
                }

                if (shadowPool->IsDynamic(request.entity)) {
                    shadowPool->SetBit<CHANGED_BIT>(request.entity);
                }

                shadowComp.version++;
            }
            });
    }

    void PointLightShadowAtlasSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto& shadowGroup = drawData->PointLightShadow;

            shadowGroup.gridLookupBuffer.Write(frameIndex, shadowGroup.gridLookupData.Data(), sizeof(uint32_t) * POINT_SHADOW_GRID_SIZE * POINT_SHADOW_GRID_SIZE, 0);
            });
    }
}