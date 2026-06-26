#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/Mesh.glsl"
#include "../../../../Includes/Common/SpotLight.glsl"

layout(location = 0) out flat uint outLightDenseIndex;
layout(location = 1) out flat uint outEntityLightIndex;
layout(location = 2) out flat uint outCameraIndex;

#include "../../../../Includes/PushConstants/DeferredSpotLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredSpotLightPC pc;
};

void main() 
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Resolve Entity ID and Sparse Indexes
    uint entityId = GET_SPOT_VISIBLE_LIGHT(ctx.spotLightVisibleIndexBufferAddr, gl_InstanceIndex);
    uint lightDenseIndex = GET_SPARSE_INDEX(ctx.spotLightSparseMapBufferAddr, entityId);   

    // 2. Vertex Data
    uint vertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    vec3 localPos = GET_VERTEX_POS(pc.vertexPositionBufferAddr, vertexIndex).position;

    // 4. Fetch Camera and Transform Vertex
    uint cameraIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraIndex);

    SpotLightComponent light = GET_SPOT_LIGHT(ctx.spotLightDataBufferAddr, lightDenseIndex);
    gl_Position = camera.viewProjVulkan * light.transform * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outEntityLightIndex = entityId;
    outCameraIndex = cameraIndex;
}