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
#include <type_traits>
#include "Pool.h"

namespace Syn
{
    template<typename T>
    struct IsSynPool : std::false_type {};

    template<typename T, typename Storage, typename Mapping>
    struct IsSynPool<Pool<T, Storage, Mapping>> : std::true_type {};

    template<typename T>
    constexpr bool IsSynPool_v = IsSynPool<T>::value;

    template<typename T>
    concept PoolConstraint = IsSynPool_v<T>;
}