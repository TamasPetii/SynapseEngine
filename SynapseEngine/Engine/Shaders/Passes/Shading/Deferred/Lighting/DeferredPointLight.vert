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

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/Mesh.glsl"
#include "../../../../Includes/Common/PointLight.glsl"

layout(location = 0) out flat uint outLightDenseIndex;
layout(location = 1) out flat uint outEntityLightIndex;
layout(location = 2) out flat uint outCameraIndex;

#include "../../../../Includes/PushConstants/DeferredPointLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredPointLightPC pc;
};

void main() 
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Resolve Entity ID and Sparse Indexes
    uint entityId = GET_POINT_VISIBLE_LIGHT(ctx.pointLightVisibleIndexBufferAddr, gl_InstanceIndex);   
    uint lightDenseIndex = GET_SPARSE_INDEX(ctx.pointLightSparseMapBufferAddr, entityId);

    // 2. Fetch Light and Vertex Data
    PointLightComponent light = GET_POINT_LIGHT(ctx.pointLightDataBufferAddr, lightDenseIndex);
    uint vertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    vec3 localPos = GET_VERTEX_POS(pc.vertexPositionBufferAddr, vertexIndex).position;

    // 3. Build Model Matrix
    mat4 model = mat4(1.0);
    model[0][0] = light.radius;
    model[1][1] = light.radius;
    model[2][2] = light.radius;
    model[3] = vec4(light.position, 1.0);
    
    // 4. Fetch Camera and Transform Vertex
    uint cameraIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraIndex);

    gl_Position = camera.viewProjVulkan * model * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outEntityLightIndex = entityId;
    outCameraIndex = cameraIndex;
}