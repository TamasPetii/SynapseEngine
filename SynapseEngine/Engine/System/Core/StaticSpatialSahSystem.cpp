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

#include "StaticSpatialSahSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include <algorithm>
#include <print>

#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"

namespace Syn
{
    std::vector<TypeID> StaticSpatialSahSystem::GetReadDependencies() const {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID
        };
    }

    std::vector<TypeID> StaticSpatialSahSystem::GetWriteDependencies() const {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<StaticSpatialSahSystem>::ID
        };
    }

    void StaticSpatialSahSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto modelPool = registry->GetPool<ModelComponent>();
        if (!transformPool || !modelPool) {
            return;
        }

        bool isEnabled = scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::StaticBvh
                      || scene->GetSettings()->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh
                      || scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh
                      || scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh;

        bool toggledOn = (isEnabled && !_wasEnabled);
        _wasEnabled = isEnabled;

        if (!isEnabled) {
            return;
        }

		auto animPool = registry->GetPool<AnimationComponent>();
        auto chunkGroup = &scene->GetSceneDrawData()->Chunks;

        bool hasDirtyStatics = !transformPool->GetStorage().GetDirtyStatics().empty();
        if (!hasDirtyStatics && !toggledOn && !chunkGroup->chunks.empty()) {
            return;
        }

        auto staticEntities = transformPool->GetStorage().GetStaticEntities();
        if (staticEntities.empty()) {
            return;
        }

        auto& modelSnapshot = scene->GetSystemContext().modelSnapshots;
        auto& animSnapshot = scene->GetSystemContext().animationSnapshots;

        _spatialItems.clear();
        _spatialItems.resize(staticEntities.size());

        chunkGroup->chunks.clear();
        chunkGroup->chunks.resize(staticEntities.size());
        chunkGroup->visibleChunkIds.resize(staticEntities.size());

        chunkGroup->chunkCounter.store(0, std::memory_order_relaxed);
        uint32_t framesInFlight = ServiceLocator::Get<FrameContext>()->framesInFlight;
        this->SetFramesToUpload(framesInFlight);
        
        auto gatherTaskOpt = this->ForEachIndex(size_t(0), staticEntities.size(), size_t(1), subflow, "GatherSpatialItems",
            [this, staticEntities, transformPool, modelPool, modelSnapshot, animPool, animSnapshot](size_t i) {
                EntityID entity = staticEntities[i];
                auto& transform = transformPool->Get(entity);

                // Fallback AABB if no valid model is attached
                glm::vec3 minB = transform.translation - transform.scale;
                glm::vec3 maxB = transform.translation + transform.scale;

                if (modelPool->Has(entity))
                {
                    const auto& modelComp = modelPool->Get(entity);

                    if (modelComp.modelIndex != NULL_INDEX && modelComp.modelIndex < modelSnapshot.size())
                    {
                        const auto& snapshotEntry = modelSnapshot[modelComp.modelIndex];

                        if (snapshotEntry.resource != nullptr && snapshotEntry.state == ResourceState::Ready)
                        {
                            GpuMeshCollider collider = snapshotEntry.resource->cpuData.globalCollider;

                            if (animPool && animPool->Has(entity))
                            {
                                const auto& animComp = animPool->Get(entity);

                                if (animComp.animationIndex != NULL_INDEX && animComp.animationIndex < animSnapshot.size())
                                {
                                    const auto& aSnapshotEntry = animSnapshot[animComp.animationIndex];
                                    if (aSnapshotEntry.resource != nullptr && aSnapshotEntry.state == ResourceState::Ready)
                                    {
                                        collider = aSnapshotEntry.resource->cpuData.globalCollider;
                                    }
                                }
                            }

                            GpuMeshCollider globalWorldCollider = MeshUtils::TransformCollider(collider, transform.transform);

                            minB = globalWorldCollider.aabbMin;
                            maxB = globalWorldCollider.aabbMax;
                        }
                    }
                }

                _spatialItems[i] = {
                    entity,
                    minB,
                    maxB,
                    (minB + maxB) * 0.5f
                };
            });

        // 2. Recursive SAH BVH builder a te EmplaceTask wrappereddel
        tf::Task sahRootTask = this->EmplaceTask(subflow, "BuildBinnedSAH_Root", [this, scene, frameIndex](tf::Subflow& sf) {
            std::span<SpatialItem> allItems(_spatialItems.data(), _spatialItems.size());
            BuildBinnedSahNodeTask(sf, scene, allItems);
            });

        // 3. Ecs Sync
        tf::Task syncEcsTask = this->EmplaceTask(subflow, "SyncECS_And_TriggerUpload", [this, transformPool, frameIndex]() {
            std::println("[StaticSpatialSah] SyncECS_And_TriggerUpload TASK fut (Frame: {})", frameIndex);
            size_t staticCount = _spatialItems.size();
            std::vector<TransformComponent> sortedTransforms(staticCount);
            std::vector<EntityID> sortedEntities(staticCount);

            for (size_t i = 0; i < staticCount; ++i) {
                EntityID entity = _spatialItems[i].entity;
                sortedTransforms[i] = transformPool->Get(entity);
                sortedEntities[i] = entity;
                sortedTransforms[i].version++;
            }

            transformPool->UpdateStaticData(std::span<const TransformComponent>(sortedTransforms));

            // Overwrite Dense Entities
            transformPool->UpdateStaticEntities(std::span<const EntityID>(sortedEntities));

            // Update Sparse Map to point to the new sorted dense indices
            transformPool->RebuildStaticIndices(std::span<const EntityID>(sortedEntities));

            transformPool->IncrementMappingVersion();

            transformPool->SetStateBitSet<FORCE_STATIC_GPU_UPLOAD>();
            });

        if (gatherTaskOpt) {
            gatherTaskOpt->precede(sahRootTask);
        }

        sahRootTask.precede(syncEcsTask);
    }

    void StaticSpatialSahSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto chunkGroup = &scene->GetSceneDrawData()->Chunks;

        if (GetFramesToUpload() == 0 || chunkGroup->chunks.empty()) {
            return;
        }

        this->EmplaceTask(subflow, "UploadChunks", [this, chunkGroup, frameIndex]() {
            size_t activeChunkCount = chunkGroup->chunkCounter.load(std::memory_order_relaxed);

            chunkGroup->chunkDataBuffer.UpdateCapacity(frameIndex, activeChunkCount);
            chunkGroup->chunkVisibilityBuffer.UpdateCapacity(frameIndex, activeChunkCount);
            chunkGroup->chunkDataBuffer.Write(frameIndex, chunkGroup->chunks.data(), activeChunkCount * sizeof(ChunkDataGPU), 0);

            DecrementFramesToUpload();

            });
    }

    void StaticSpatialSahSystem::BuildBinnedSahNodeTask(tf::Subflow& subflow, Scene* scene, std::span<SpatialItem> items)
    {
        auto chunkGroup = &scene->GetSceneDrawData()->Chunks;

        // Base Case: Leaf node reached, chunk size constraint met
        if (items.size() <= CHUNK_MAX_SIZE)
        {
            glm::vec3 nodeMin(FLT_MAX);
            glm::vec3 nodeMax(-FLT_MAX);

            // Compute tight Macro-AABB for this leaf chunk
            for (const auto& item : items) {
                nodeMin = glm::min(nodeMin, item.minBounds);
                nodeMax = glm::max(nodeMax, item.maxBounds);
            }

            // Lock-free atomic chunk allocation
            uint32_t chunkIndex = chunkGroup->chunkCounter.fetch_add(1, std::memory_order_relaxed);
            uint32_t startIndex = static_cast<uint32_t>(items.data() - _spatialItems.data());

            chunkGroup->chunks[chunkIndex] = {
                nodeMin,
                startIndex,
                nodeMax,
                static_cast<uint32_t>(items.size())
            };
            return;
        }

        // Calculate the overall bounds of centroids to find the longest axis
        glm::vec3 centroidMin(FLT_MAX);
        glm::vec3 centroidMax(-FLT_MAX);

        for (const auto& item : items) {
            centroidMin = glm::min(centroidMin, item.centroid);
            centroidMax = glm::max(centroidMax, item.centroid);
        }

        // Determine split axis (X=0, Y=1, Z=2) based on maximum spatial spread
        glm::vec3 extent = centroidMax - centroidMin;
        int splitAxis = 0;
        if (extent.y > extent.x) splitAxis = 1;
        if (extent.z > extent[splitAxis]) splitAxis = 2;

        // Edge Case: All items share the exact same centroid. Force split in half to avoid infinite recursion.
        if (extent[splitAxis] < 1e-4f) {
            size_t mid = items.size() / 2;
            subflow.emplace([this, scene, items, mid](tf::Subflow& sf) { BuildBinnedSahNodeTask(sf, scene, items.subspan(0, mid)); }).name("SAH_Fallback_Left");
            subflow.emplace([this, scene, items, mid](tf::Subflow& sf) { BuildBinnedSahNodeTask(sf, scene, items.subspan(mid)); }).name("SAH_Fallback_Right");
            return;
        }

        SahBin bins[SAH_NUM_BINS];
        float scale = SAH_NUM_BINS / extent[splitAxis];

        // Populate Bins: Map each item's centroid to a specific bin
        for (const auto& item : items) {
            int binIdx = std::min(static_cast<int>(SAH_NUM_BINS - 1), static_cast<int>((item.centroid[splitAxis] - centroidMin[splitAxis]) * scale));
            bins[binIdx].count++;
            bins[binIdx].minBounds = glm::min(bins[binIdx].minBounds, item.minBounds);
            bins[binIdx].maxBounds = glm::max(bins[binIdx].maxBounds, item.maxBounds);
        }

        // Evaluate SAH cost for all possible split planes
        float leftArea[SAH_NUM_BINS - 1], rightArea[SAH_NUM_BINS - 1];
        int leftCount[SAH_NUM_BINS - 1], rightCount[SAH_NUM_BINS - 1];

        SahBin leftAccumulator, rightAccumulator;

        // Forward sweep: Accumulate bounds and counts from left to right
        for (int i = 0; i < SAH_NUM_BINS - 1; ++i) {
            leftAccumulator.count += bins[i].count;
            leftAccumulator.minBounds = glm::min(leftAccumulator.minBounds, bins[i].minBounds);
            leftAccumulator.maxBounds = glm::max(leftAccumulator.maxBounds, bins[i].maxBounds);

            leftCount[i] = leftAccumulator.count;
            glm::vec3 e = glm::max(glm::vec3(0.0f), leftAccumulator.maxBounds - leftAccumulator.minBounds);
            leftArea[i] = e.x * e.y + e.y * e.z + e.z * e.x;
        }

        // Backward sweep: Accumulate bounds and counts from right to left
        for (int i = SAH_NUM_BINS - 1; i > 0; --i) {
            rightAccumulator.count += bins[i].count;
            rightAccumulator.minBounds = glm::min(rightAccumulator.minBounds, bins[i].minBounds);
            rightAccumulator.maxBounds = glm::max(rightAccumulator.maxBounds, bins[i].maxBounds);

            rightCount[i - 1] = rightAccumulator.count;
            glm::vec3 e = glm::max(glm::vec3(0.0f), rightAccumulator.maxBounds - rightAccumulator.minBounds);
            rightArea[i - 1] = e.x * e.y + e.y * e.z + e.z * e.x;
        }

        // Find the split plane with the minimum SAH cost
        float minCost = FLT_MAX;
        int bestSplit = -1;
        for (int i = 0; i < SAH_NUM_BINS - 1; ++i) {
            float cost = leftCount[i] * leftArea[i] + rightCount[i] * rightArea[i];
            if (cost < minCost) {
                minCost = cost;
                bestSplit = i;
            }
        }

        // In-place partition: Reorder items in memory based on the best split plane
        auto splitIter = std::partition(items.begin(), items.end(), [=](const SpatialItem& item) {
            int binIdx = std::min(static_cast<int>(SAH_NUM_BINS - 1), static_cast<int>((item.centroid[splitAxis] - centroidMin[splitAxis]) * scale));
            return binIdx <= bestSplit;
            });

        size_t midIndex = std::distance(items.begin(), splitIter);

        // Safety fallback: If partitioning fails to split
        if (midIndex == 0 || midIndex == items.size()) {
            midIndex = items.size() / 2;
        }

        // Spawn child tasks for the left and right sub-trees
        subflow.emplace([this, scene, items, midIndex](tf::Subflow& sf) { BuildBinnedSahNodeTask(sf, scene, items.subspan(0, midIndex)); }).name("SAH_Left");
        subflow.emplace([this, scene, items, midIndex](tf::Subflow& sf) { BuildBinnedSahNodeTask(sf, scene, items.subspan(midIndex)); }).name("SAH_Right");
    }
}