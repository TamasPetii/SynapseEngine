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
#include <limits>
#include <functional>

namespace Syn {
    using EntityID = uint32_t;
    using DenseIndex = uint32_t;

    constexpr EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();
    constexpr DenseIndex NULL_INDEX = std::numeric_limits<DenseIndex>::max();

    using SwapCallback = std::function<void(EntityID movedEntity, DenseIndex newIndex)>;
}