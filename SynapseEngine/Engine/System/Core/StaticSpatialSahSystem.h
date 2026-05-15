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

    struct SYN_API SahBin {
        uint32_t count = 0;
        glm::vec3 minBounds = glm::vec3(FLT_MAX);
        glm::vec3 maxBounds = glm::vec3(-FLT_MAX);
    };

    class SYN_API StaticSpatialSahSystem : public ISystem
    {
    public:
        static constexpr size_t SAH_NUM_BINS = 16;
        static constexpr size_t CHUNK_MAX_SIZE = 64;

        std::string GetName() const override { return "StaticSpatialSahSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void BuildStaticHierarchy(Scene* scene, tf::Taskflow& taskflow);

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
    private:
        void BuildBinnedSahNodeTask(tf::Subflow& subflow, Scene* scene, std::span<SpatialItem> items);
        std::vector<SpatialItem> _spatialItems;
        std::atomic<uint32_t> _uploadCountdown{ 0 };
    };
}