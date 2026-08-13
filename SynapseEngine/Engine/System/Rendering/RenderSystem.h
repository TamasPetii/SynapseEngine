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
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Mesh/MeshAllocationInfo.h"

namespace Syn
{
    struct SYN_API MeshMatCapacity {
        uint32_t capacities[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount] = { 0 };
    };

    class SYN_API RenderSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "RenderSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::RenderingSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;
    private:
        void RebuildGlobalBuffers(Scene* scene);
        MaterialRenderType GetMaterialType(uint32_t materialIndex) const;
    private:
        bool _needsRebuild = true;
        uint32_t _lastModelManagerVersion = 0;
        uint32_t _lastMaterialManagerVersion = 0;

        std::vector<uint32_t> _modelCapacities;
        std::vector<std::vector<EntityID>> _entitiesPerModel;
        std::vector<std::vector<MeshMatCapacity>> _meshMatCapacities;
    };
}