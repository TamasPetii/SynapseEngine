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

#ifndef SYN_INCLUDES_UTILS_ENVIRONMENT_MATH_GLSL
#define SYN_INCLUDES_UTILS_ENVIRONMENT_MATH_GLSL

vec2 SampleEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.15915494309, 0.31830988618);
    uv += 0.5;
    return uv;
}

vec2 SampleOctahedral(vec3 v) {
    v /= (abs(v.x) + abs(v.y) + abs(v.z));
    vec2 uv = v.z >= 0.0 ? v.xy : (1.0 - abs(v.yx)) * sign(v.xy);
    return uv * 0.5 + 0.5;
}

#endif