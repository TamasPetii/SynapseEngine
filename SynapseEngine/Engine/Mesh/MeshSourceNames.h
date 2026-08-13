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
    struct SYN_API MeshSourceNames
    {
        static constexpr const char* Cube = "Cube";
        static constexpr const char* Sphere = "Sphere";
        static constexpr const char* ProxySphere = "ProxySphere";
        static constexpr const char* Quad = "Quad";
        static constexpr const char* ScreenQuad = "ScreenQuad";
        static constexpr const char* Cylinder = "Cylinder";
        static constexpr const char* Cone = "Cone";
        static constexpr const char* ProxyCone = "ProxyCone";
        static constexpr const char* Capsule = "Capsule";
        static constexpr const char* Hemisphere = "Hemisphere";
        static constexpr const char* Pyramid = "Pyramid";
        static constexpr const char* ProxyPyramid = "ProxyPyramid";
        static constexpr const char* Grid = "Grid";
        static constexpr const char* Torus = "Torus";
        static constexpr const char* IcoSphere = "IcoSphere";
        static constexpr const char* ProxyIcoSphere = "ProxyIcoSphere";
    };
}