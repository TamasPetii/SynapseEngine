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

#ifndef SYN_INCLUDES_COMMON_SPOT_LIGHT_GLSL
#define SYN_INCLUDES_COMMON_SPOT_LIGHT_GLSL

#include "../Core.glsl"

struct SpotLightComponent {
    mat4 transform;
    vec3 position;
    float range;
    vec3 direction;
    float weakenDistance;
    vec3 color;
    float strength;
    float innerAngle;
    float outerAngle;
    float innerCosAngle;
    float outerCosAngle;
    uint flags;
    uint padding0;
    uint padding1;
    uint padding2;
};

struct SpotLightColliderGPU {
    vec3 center;
    float radius;
    vec3 aabbMin;
    uint entityIndex;
    vec3 aabbMax;
    float range;
    vec3 worldPos;
    float outerAngleCos;
    vec3 worldDir;
    float outerAngleSin;
};

struct SpotLightShadowComponent {
    vec4 planes;
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec4 atlasRect;
};

layout(buffer_reference, std430) readonly restrict buffer SpotLightDataBuffer { SpotLightComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotLightColliderDataBuffer { SpotLightColliderGPU data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotLightShadowDataBuffer { SpotLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotShadowInstanceBuffer { uvec2 data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotVisibleCountBuffer { uint data; };
layout(buffer_reference, std430) readonly restrict buffer SpotVisibleLightBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotDrawCallKeyBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotSortValuesBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotGridLookupBuffer { uint data[]; };

#define SPOT_SHADOW_ATLAS_SIZE 4096
#define SPOT_SHADOW_MIN_BLOCK_SIZE 64
#define SPOT_SHADOW_GRID_SIZE (SPOT_SHADOW_ATLAS_SIZE / SPOT_SHADOW_MIN_BLOCK_SIZE)

#define GET_SPOT_LIGHT(addr, idx)                       SpotLightDataBuffer(addr).data[idx]
#define GET_SPOT_LIGHT_COLLIDER(addr, idx)              SpotLightColliderDataBuffer(addr).data[idx]
#define GET_SPOT_LIGHT_SHADOW(addr, idx)                SpotLightShadowDataBuffer(addr).data[idx]
#define GET_SPOT_VISIBLE_LIGHT(addr, idx)               SpotVisibleLightBuffer(addr).data[idx]
#define GET_SPOT_SHADOW_INSTANCE(addr, idx)             SpotShadowInstanceBuffer(addr).data[idx]
#define GET_SPOT_VISIBLE_SHADOW_LIGHT(addr, idx)        SpotVisibleLightBuffer(addr).data[idx]
#define GET_SPOT_GRID_LOOK_UP_DATA(addr, idx)           SpotGridLookupBuffer(addr).data[idx]
#define GET_SPOT_VISIBLE_COUNT_DATA(addr)               SpotVisibleCountBuffer(addr).data
#define GET_SPOT_DRAW_CALL_KEY_DATA(addr, idx)          SpotDrawCallKeyBuffer(addr).data[idx]
#define GET_SPOT_SORTED_VALUE(addr, idx)                SpotSortValuesBuffer(addr).data[idx]
#define GET_SPOT_SHADOW_INSTANCE_UNSORTED(addr, idx)    SpotShadowInstanceBuffer(addr).data[idx]

#define GET_SPOT_ATLAS_SORT_KEY(addr, idx)                   SpotDrawCallKeyBuffer(addr).data[idx]
#define GET_SPOT_ATLAS_SORT_VALUE(addr, idx)                 SpotSortValuesBuffer(addr).data[idx]
#endif