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

#version 460

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D accumTex;
layout(set = 2, binding = 1) uniform sampler2D revealTex;

void main() {
    float revealage = texture(revealTex, inUV).r;
    
    if (revealage == 1.0) 
        discard;

    vec4 accum = texture(accumTex, inUV);
    vec3 averageColor = accum.rgb / max(accum.a, 1e-5);

    outColor = vec4(averageColor * (1.0 - revealage), revealage);
}