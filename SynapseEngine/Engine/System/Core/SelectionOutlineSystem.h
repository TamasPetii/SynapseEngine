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
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include <vector>

namespace Syn
{
    class SYN_API SelectionOutlineSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "SelectionOutlineSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;

        std::vector<TypeID> GetWriteDependencies() const override;
    private:
        EntityID _lastSelectedEntity = NULL_ENTITY;
        uint64_t _lastHierarchyVersion = 0;
        std::vector<uint32_t> _selectionMask;
    };
}