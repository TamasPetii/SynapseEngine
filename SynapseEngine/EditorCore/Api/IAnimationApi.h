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
#include <cstdint>
#include <vector>
#include <string>
#include "IApi.h"
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"

namespace Syn
{
    constexpr uint32_t INVALID_ANIMATION_ID = 0xFFFFFFFF;

    struct AnimationItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IAnimationApi : public IApi {
    public:
        virtual ~IAnimationApi() = default;

        virtual std::vector<AnimationItemData> GetAllAnimations() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t animationId) = 0;
        virtual uint32_t GetSelected() const = 0;

        virtual const CpuAnimationData* GetAnimationCpuData(uint32_t animationId) const = 0;

        virtual void ApplyAnimationToPreviewObject(uint32_t animationId) = 0;
    };
}