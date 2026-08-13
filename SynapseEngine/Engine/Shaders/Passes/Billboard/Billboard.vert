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
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Transform.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out flat uint outEntityId;

#include "../../Includes/PushConstants/BillboardPC.glsl"

layout(push_constant) uniform PushConstants {
    BillboardPC pc;
};

const vec2 QUAD_POSITIONS[6] = vec2[](
    vec2(-0.5, -0.5), vec2( 0.5, -0.5), vec2(-0.5,  0.5),
    vec2(-0.5,  0.5), vec2( 0.5, -0.5), vec2( 0.5,  0.5)
);

const vec2 QUAD_UVS[6] = vec2[](
    vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(0.0, 0.0),
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(1.0, 0.0)
);

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer VisibleEntityBuffer {
    uint data[];
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint entityId = VisibleEntityBuffer(pc.visibleEntitiesAddr).data[gl_InstanceIndex];

    if (entityId == ctx.activeCameraEntity) {
        gl_Position = vec4(0.0);
        return;
    }

    uint transformDenseIdx = GET_SPARSE_INDEX(ctx.transformSparseMapBufferAddr, entityId);
    TransformComponent transform = GET_TRANSFORM(ctx.transformBufferAddr, transformDenseIdx);
    vec3 worldCenter = transform.transform[3].xyz;

    uint activeCamDenseIdx = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent activeCamera = GET_CAMERA(ctx.cameraBufferAddr, activeCamDenseIdx);

    vec3 camRight = vec3(activeCamera.view[0][0], activeCamera.view[1][0], activeCamera.view[2][0]);
    vec3 camUp    = vec3(activeCamera.view[0][1], activeCamera.view[1][1], activeCamera.view[2][1]);

    vec4 viewPos = activeCamera.view * vec4(worldCenter, 1.0);
    float dist = abs(viewPos.z); 
    float finalSize = pc.baseScale * dist * 0.05;

    vec2 localPos = QUAD_POSITIONS[gl_VertexIndex % 6];
    
    vec3 vertexWorldPos = worldCenter 
                        + (camRight * localPos.x * finalSize) 
                        + (camUp    * localPos.y * finalSize);

    gl_Position = activeCamera.viewProjVulkan * vec4(vertexWorldPos, 1.0);

    outUV = QUAD_UVS[gl_VertexIndex % 6];
    outEntityId = entityId;
}