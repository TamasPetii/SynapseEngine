#pragma once
#include "Engine/System/ISystem.h"
#include <vector>
#include <atomic>
#include <span>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API SpatialItem {
        EntityID entity;
        glm::vec3 minBounds;
        glm::vec3 maxBounds;
        glm::vec3 centroid;
    };

    struct SYN_API ChunkDataGPU {
        glm::vec3 minBounds;
        uint32_t  firstEntityIndex;
        glm::vec3 maxBounds;
        uint32_t  entityCount;
    };

    struct SYN_API SahBin {
        uint32_t count = 0;
        glm::vec3 minBounds = glm::vec3(FLT_MAX);
        glm::vec3 maxBounds = glm::vec3(-FLT_MAX);
    };

    struct SYN_API StaticEntityLink {
        uint32_t entityID;
        uint32_t modelDenseIndex;
    };

    class SYN_API StaticSpatialSahSystem : public ISystem
    {
    public:
        static constexpr size_t SAH_NUM_BINS = 16;
        static constexpr size_t CHUNK_MAX_SIZE = 64;

        std::string GetName() const override { return "StaticSpatialSahSystem"; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void BuildStaticHierarchy(Scene* scene, tf::Taskflow& taskflow);

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
    private:
        void BuildBinnedSahNodeTask(tf::Subflow& subflow, std::span<SpatialItem> items);

        std::atomic<uint32_t> _chunkCounter;
        std::vector<ChunkDataGPU> _chunks;
        std::vector<SpatialItem> _spatialItems;

        bool _needsChunkUpload = false;
    };
}