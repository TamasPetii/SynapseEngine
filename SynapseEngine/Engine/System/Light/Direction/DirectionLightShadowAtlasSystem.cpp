#include "DirectionLightShadowAtlasSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Logger/SynLog.h"
#include "DirectionLightCullingSystem.h"
#include "Engine/Scene/DrawData/SceneDrawData.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include <array>

namespace Syn
{
    constexpr bool ENABLE_ATLAS_DEBUG_LOGGING = false;

    std::vector<TypeID> DirectionLightShadowAtlasSystem::GetReadDependencies() const {
        return {
            TypeInfo<DirectionLightCullingSystem>::ID
        };
    }

    std::vector<TypeID> DirectionLightShadowAtlasSystem::GetWriteDependencies() const {
        return {
            TypeInfo<DirectionLightShadowAtlasSystem>::ID
        };
    }

    void DirectionLightShadowAtlasSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();

        if (!shadowPool) return;

        this->EmplaceTask(subflow, "Update Shadow Atlas", [drawData, shadowPool]() {

            uint32_t activeLights = drawData->DirectionLightShadow.visibleLightCount;
            if (activeLights == 0) 
                return;

            //Initialize 2D allocation grid (false = free, true = occupied)
            std::array<std::array<bool, SHADOW_GRID_SIZE>, SHADOW_GRID_SIZE> grid = { false };

            //Calculate dynamic cascade resolution based on active light count
            uint32_t totalCascades = activeLights * CASCADES_PER_LIGHT;
            uint32_t cascadesPerRow = static_cast<uint32_t>(std::ceil(std::sqrt(totalCascades)));

            // Calculate max available size per cascade and snap to grid blocks
            uint32_t cascadeSizePx = SHADOW_ATLAS_SIZE / cascadesPerRow;
            cascadeSizePx = (cascadeSizePx / SHADOW_MIN_BLOCK_SIZE) * SHADOW_MIN_BLOCK_SIZE;
            uint32_t blockSize = cascadeSizePx / SHADOW_MIN_BLOCK_SIZE;


            //Finds contiguous free blocks in the 2D grid and reserves them
            auto AllocateBlock = [&](uint32_t size, uint32_t& outX, uint32_t& outY) -> bool {
                for (uint32_t y = 0; y <= SHADOW_GRID_SIZE - size; ++y) {
                    for (uint32_t x = 0; x <= SHADOW_GRID_SIZE - size; ++x) {

                        // Check if the required NxN area is entirely free
                        bool free = true;
                        for (uint32_t by = 0; by < size; ++by) {
                            for (uint32_t bx = 0; bx < size; ++bx) {
                                if (grid[y + by][x + bx]) {
                                    free = false; 
                                    break;
                                }
                            }
                            if (!free) 
                                break;
                        }

                        // If free, mark the NxN area as occupied and return grid coordinates
                        if (free) {
                            for (uint32_t by = 0; by < size; ++by) {
                                for (uint32_t bx = 0; bx < size; ++bx) {
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

            // Allocate regions for all visible light cascades
            for (uint32_t lightIdx = 0; lightIdx < activeLights; ++lightIdx)
            {
                EntityID entity = drawData->DirectionLightShadow.visibleLights[lightIdx];
                auto& shadowComp = shadowPool->Get(entity);

                for (uint32_t cascadeIdx = 0; cascadeIdx < CASCADES_PER_LIGHT; ++cascadeIdx)
                {
                    uint32_t gridX = 0, gridY = 0;

                    if (AllocateBlock(blockSize, gridX, gridY))
                    {
                        // Convert grid coordinates to normalized [0.0, 1.0] UV space for the shader
                        float uvX = static_cast<float>(gridX) / SHADOW_GRID_SIZE;
                        float uvY = static_cast<float>(gridY) / SHADOW_GRID_SIZE;
                        float uvW = static_cast<float>(blockSize) / SHADOW_GRID_SIZE;
                        float uvH = static_cast<float>(blockSize) / SHADOW_GRID_SIZE;

                        shadowComp.cascadeAtlasRects[cascadeIdx] = glm::vec4(uvX, uvY, uvW, uvH);

                        if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                            Info("Atlas Alloc - Light: {}, Cascade: {} -> X: {}, Y: {}, Size: {}x{}",
                                 lightIdx, cascadeIdx,
                                 gridX * SHADOW_MIN_BLOCK_SIZE,
                                 gridY * SHADOW_MIN_BLOCK_SIZE,
                                 cascadeSizePx, cascadeSizePx);
                        }
                    }
                    else
                    {
                        shadowComp.cascadeAtlasRects[cascadeIdx] = glm::vec4(0.0f);

                        if constexpr (ENABLE_ATLAS_DEBUG_LOGGING) {
                            Error("Atlas Full! Could not allocate Cascade {} for Light {}", cascadeIdx, lightIdx);
                        }
                    }
                }

                if (shadowPool->IsDynamic(entity)) {
                    shadowPool->SetBit<CHANGED_BIT>(entity);
                }

                shadowComp.version++;
            }
            });
    }
}