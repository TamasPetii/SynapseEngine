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
#include "Engine/Component/Rendering/AnimationComponent.h"

namespace Syn
{
    class SYN_API AnimationSystem : public ComponentSystem<AnimationComponent>
    {
    public:
        std::string GetName() const override { return "AnimationSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::RenderingSystems; }

        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::AnimationSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic) override;
    };
}