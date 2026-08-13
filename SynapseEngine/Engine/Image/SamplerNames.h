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

namespace Syn
{
    struct SYN_API SamplerNames
    {
        static constexpr const char* LinearRepeat = "LinearRepeat";
        static constexpr const char* LinearClampEdge = "LinearClampEdge";
        static constexpr const char* NearestRepeat = "NearestRepeat";
        static constexpr const char* NearestClampEdge = "NearestClampEdge";
        static constexpr const char* LinearAniso = "LinearAniso";
        static constexpr const char* NearestAniso = "NearestAniso";
        static constexpr const char* MaxReduction = "MaxReduction";
        static constexpr const char* BloomSampler = "BloomSampler";
        static constexpr const char* ShadowSampler = "ShadowSampler";
        static constexpr const char* SkyboxSampler = "SkyboxSampler";
    };

    struct SYN_API SamplerWrapModeNames
    {
        static constexpr const char* Repeat = "Repeat";
        static constexpr const char* ClampEdge = "ClampEdge";
    };
}