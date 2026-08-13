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
        static constexpr size_t SAH_NUM_BINS = 8;
        static constexpr size_t CHUNK_MAX_SIZE = 32;

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
        bool _wasEnabled = false;
    };
}