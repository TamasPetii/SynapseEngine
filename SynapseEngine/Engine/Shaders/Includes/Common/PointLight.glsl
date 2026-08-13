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

#ifndef SYN_INCLUDES_COMMON_POINT_LIGHT_GLSL
#define SYN_INCLUDES_COMMON_POINT_LIGHT_GLSL

#include "../Core.glsl"

struct PointLightComponent {
    vec3 position;
    float radius;
    vec3 color;
    float strength;
    float weakenDistance;
    uint flags;
    uint padding;
    uint padding2;
};

struct PointLightColliderGPU
{
    vec3 center;
    float radius;
    vec3 padding;
    uint entityIndex;
};

struct PointLightShadowComponent {
    vec4 planes;
    vec4 mainAtlasRect;
    mat4 viewProjs[6];
    vec4 atlasRects[6];
};

layout(buffer_reference, std430) readonly restrict buffer PointLightDataBuffer { PointLightComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointLightColliderDataBuffer { PointLightColliderGPU data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointLightShadowDataBuffer { PointLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointVisibleLightBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointShadowInstanceBuffer { uvec2 data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointGridLookupBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointVisibleCountBuffer { uint data; };
layout(buffer_reference, std430) readonly restrict buffer PointDrawCallKeyBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer PointSortValuesBuffer { uint data[]; };

#define POINT_SHADOW_ATLAS_SIZE 4096
#define POINT_SHADOW_MIN_BLOCK_SIZE 64
#define POINT_SHADOW_GRID_SIZE (POINT_SHADOW_ATLAS_SIZE / POINT_SHADOW_MIN_BLOCK_SIZE)

#define GET_POINT_LIGHT(addr, idx)                       PointLightDataBuffer(addr).data[idx]
#define GET_POINT_LIGHT_COLLIDER(addr, idx)              PointLightColliderDataBuffer(addr).data[idx]
#define GET_POINT_LIGHT_SHADOW(addr, idx)                PointLightShadowDataBuffer(addr).data[idx]
#define GET_POINT_VISIBLE_LIGHT(addr, idx)               PointVisibleLightBuffer(addr).data[idx]
#define GET_POINT_SHADOW_INSTANCE(addr, idx)             PointShadowInstanceBuffer(addr).data[idx]
#define GET_POINT_VISIBLE_SHADOW_LIGHT(addr, idx)        PointVisibleLightBuffer(addr).data[idx]
#define GET_POINT_GRID_LOOK_UP_DATA(addr, idx)           PointGridLookupBuffer(addr).data[idx]
#define GET_POINT_VISIBLE_COUNT_DATA(addr)               PointVisibleCountBuffer(addr).data
#define GET_POINT_DRAW_CALL_KEY_DATA(addr, idx)          PointDrawCallKeyBuffer(addr).data[idx]
#define GET_POINT_SORTED_VALUE(addr, idx)                PointSortValuesBuffer(addr).data[idx]
#define GET_POINT_SHADOW_INSTANCE_UNSORTED(addr, idx)    PointShadowInstanceBuffer(addr).data[idx]

#define GET_POINT_ATLAS_SORT_KEY(addr, idx)              PointDrawCallKeyBuffer(addr).data[idx]
#define GET_POINT_ATLAS_SORT_VALUE(addr, idx)            PointSortValuesBuffer(addr).data[idx]

#endif