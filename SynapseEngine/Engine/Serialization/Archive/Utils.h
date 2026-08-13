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
#include <cstdint>
#include <cstddef>

namespace Syn
{
    template <typename Archive>
    struct ScopedArchiveObject {
        ScopedArchiveObject(Archive& ar, const char* name) : _ar(ar) { _ar.EnterObject(name); }
        ~ScopedArchiveObject() { _ar.LeaveObject(); }
        Archive& _ar;
    };

    template <typename Archive>
    struct ScopedArchiveArray {
        ScopedArchiveArray(Archive& ar, const char* name, size_t size) : _ar(ar) { _ar.EnterArray(name, size); }
        ~ScopedArchiveArray() { _ar.LeaveArray(); }
        Archive& _ar;
    };
}