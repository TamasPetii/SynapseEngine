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
#include "Engine/SynApi.h"
#include "Engine/Serialization/Archive/Utils.h"
#include <utility>
#include <type_traits>

namespace Syn
{
    template <typename T>
    struct ComponentName;

    #define SYN_REGISTER_COMPONENT(Type, Name) \
        template <> struct ComponentName<Type> { \
            static constexpr const char* value = Name; \
        }

    template <typename T, typename Enable = void>
    struct Schema {
        static constexpr bool exists = false;
    };

    template <typename T, typename Archive = void>
    concept has_schema = Schema<std::remove_cvref_t<T>>::exists;
}