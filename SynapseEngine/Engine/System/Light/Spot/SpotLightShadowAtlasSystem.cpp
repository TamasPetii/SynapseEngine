#include "SpotLightShadowAtlasSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Logger/SynLog.h"
#include "SpotLightCullingSystem.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Collision/Tester/CollisionTester.h"
#include <array>
#include <vector>
#include <algorithm>

namespace Syn
{
    constexpr bool ENABLE_ATLAS_DEBUG_LOGGING = false;

    struct SpotLightAllocData {
        EntityID entity;
        uint32_t blockSizePx;
        uint32_t blocksRequired;
    };

    std::vector<TypeID> SpotLightShadowAtlasSystem::GetReadDependencies() const {
        return {
            TypeInfo<SpotLightCullingSystem>::ID
        };
    }

    std::vector<TypeID> SpotLightShadowAtlasSystem::GetWriteDependencies() const {
        return {
            TypeInfo<SpotLightShadowAtlasSystem>::ID
        };
    }

    void SpotLightShadowAtlasSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto lightPool = registry->GetPool<SpotLightComponent>();
        auto shadowPool = registry->GetPool<SpotLightShadowComponent>();
        auto cameraPool = registry->GetPool<CameraComponent>();
        EntityID cameraEntity = scene->GetSceneCameraEntity();

        if (!shadowPool || !lightPool || !cameraPool || cameraEntity == NULL_ENTITY) return;

        this->EmplaceTask(subflow, "Update Spot Shadow Atlas", [drawData, lightPool, shadowPool, cameraPool, cameraEntity]() {

            uint32_t activeLights = drawData->SpotLightShadow.visibleLightCount;

            if (activeLights == 0)
                return;

            std::fill(drawData->SpotLightShadow.gridLookupData.Data(),
                drawData->SpotLightShadow.gridLookupData.Data() + (SPOT_SHADOW_GRID_SIZE * SPOT_SHADOW_GRID_SIZE),
                0xFFFFFFFF);

            const auto& cameraComp = cameraPool->Get(cameraEntity);
            glm::vec2 screenRes = glm::vec2(cameraComp.width, cameraComp.height);

            std::vector<SpotLightAllocData> allocRequests;
            allocRequests.reserve(activeLights);

            // Evaluate screen space size for each visible shadow-casting spot light
            for (uint32_t i = 0; i < activeLights; ++i)
            {
                EntityID entity = drawData->SpotLightShadow.visibleLights[i];
                const auto& lightComp = lightPool->Get(entity);

                float screenSizePixels = CollisionTester::CalculateSphereScreenSize(
                    lightComp.sphereCollider.center, lightComp.sphereCollider.radius,
                    cameraComp.view, cameraComp.proj, cameraComp.nearPlane, screenRes);

                uint32_t blockSizePx = SPOT_SHADOW_MIN_BLOCK_SIZE;
                if (screenSizePixels > 1024.0f) blockSizePx = 1024;
                else if (screenSizePixels > 512.0f) blockSizePx = 512;
                else if (screenSizePixels > 256.0f) blockSizePx = 256;
                else if (screenSizePixels > 128.0f) blockSizePx = 128;

                allocRequests.push_back({
                    entity,
                    blockSizePx,
                    blockSizePx / SPOT_SHADOW_MIN_BLOCK_SIZE
                    });
            }

            // Greedy bin packing: Sort requests descending by required block size
            std::sort(allocRequests.begin(), allocRequests.end(), [](const SpotLightAllocData& a, const SpotLightAllocData& b) {
                return a.blockSizePx > b.blockSizePx;
                });

            std::array<std::array<bool, SPOT_SHADOW_GRID_SIZE>, SPOT_SHADOW_GRID_SIZE> grid = { false };

            auto AllocateBlock = [&](uint32_t sizeBlocks, uint32_t& outX, uint32_t& outY) -> bool {
                for (uint32_t y = 0; y <= SPOT_SHADOW_GRID_SIZE - sizeBlocks; y += 1) {
                    for (uint32_t x = 0; x <= SPOT_SHADOW_GRID_SIZE - sizeBlocks; x += 1) {

                        bool isFree = true;
                        for (uint32_t by = 0; by < sizeBlocks; ++by) {
                            for (uint32_t bx = 0; bx < sizeBlocks; ++bx) {
                                if (grid[y + by][x + bx]) {
                                    isFree = false;
                                    break;
                                }
                            }
                            if (!isFree) break;
                        }

                        if (isFree) {
                            for (uint32_t by = 0; by < sizeBlocks; ++by) {
                                for (uint32_t bx = 0; bx < sizeBlocks; ++bx) {
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

            //Allocate regions and write back to components
            for (const auto& request : allocRequests)
            {
                auto& shadowComp = shadowPool->Get(request.entity);
                uint32_t gridX = 0, gridY = 0;

                if (AllocateBlock(request.blocksRequired, gridX, gridY))
                {
                    float uvX = static_cast<float>(gridX) / SPOT_SHADOW_GRID_SIZE;
                    float uvY = static_cast<float>(gridY) / SPOT_SHADOW_GRID_SIZE;
                    float uvW = static_cast<float>(request.blocksRequired) / SPOT_SHADOW_GRID_SIZE;
                    float uvH = static_cast<float>(request.blocksRequired) / SPOT_SHADOW_GRID_SIZE;

                    shadowComp.atlasRect = glm::vec4(uvX, uvY, uvW, uvH);

                    for (uint32_t by = 0; by < request.blocksRequired; ++by) {
                        for (uint32_t bx = 0; bx < request.blocksRequired; ++bx) {
                            uint32_t flatIndex = (gridY + by) * SPOT_SHADOW_GRID_SIZE + (gridX + bx);
                            drawData->SpotLightShadow.gridLookupData[flatIndex] = static_cast<uint32_t>(request.entity);
                        }
                    }

                    if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                        Info("SpotLight Atlas Alloc - Entity: {} -> X: {}, Y: {}, Size: {}x{}",
                            static_cast<uint32_t>(request.entity),
                            gridX * SPOT_SHADOW_MIN_BLOCK_SIZE,
                            gridY * SPOT_SHADOW_MIN_BLOCK_SIZE,
                            request.blockSizePx, request.blockSizePx);
                    }
                }
                else
                {
                    shadowComp.atlasRect = glm::vec4(0.0f);

                    if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                        Warning("SpotLight Atlas Full! Dropped shadow for entity {}.", static_cast<uint32_t>(request.entity));
                    }
                }

                if (shadowPool->IsDynamic(request.entity)) {
                    shadowPool->SetBit<CHANGED_BIT>(request.entity);
                }

                shadowComp.version++;
            }
            });
    }

    void SpotLightShadowAtlasSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [scene, frameIndex]() {
            auto drawData = scene->GetSceneDrawData();
            auto& shadowGroup = drawData->SpotLightShadow;

            if (auto mapped = shadowGroup.gridLookupBuffer.GetMapped(frameIndex)) {
                mapped->Write(shadowGroup.gridLookupData.Data(), sizeof(uint32_t) * SPOT_SHADOW_GRID_SIZE * SPOT_SHADOW_GRID_SIZE, 0);
            }
            });
    }
}