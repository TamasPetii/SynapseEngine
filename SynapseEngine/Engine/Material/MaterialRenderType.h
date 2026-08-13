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

namespace Syn
{
    enum MaterialRenderType : uint32_t {
        Opaque1Sided = 0,
        Opaque2Sided = 1,
        AlphaTestedOpaque1Sided = 2,
        AlphaTestedOpaque2Sided = 3,
        Transparent1Sided = 4,
        Transparent2Sided = 5,
        AlphaTestedTransparent1Sided = 6,
        AlphaTestedTransparent2Sided = 7,
        MaterialRenderTypeCount = 8
    };
}