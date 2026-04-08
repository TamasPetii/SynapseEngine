#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/SpotLight.glsl"

layout(location = 0) out flat uint outLightDenseIndex;
layout(location = 1) out flat uint outShadowDenseIndex;
layout(location = 2) out flat uint outCameraIndex;

#include "../../Includes/PushConstants/DeferredSpotLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredSpotLightPC pc;
};

void main() 
{
    // 1. Resolve Entity ID and Sparse Indexes
    uint entityId = GET_VISIBLE_SPOT_LIGHT(pc.visibleLightAddr, gl_InstanceIndex);
    uint lightDenseIndex = GET_SPARSE_INDEX(pc.spotLightSparseMapAddr, entityId);   
    uint shadowDenseIndex = INVALID_INDEX; 

    if (pc.spotLightShadowSparseMapAddr != 0) {
        shadowDenseIndex = GET_SPARSE_INDEX(pc.spotLightShadowSparseMapAddr, entityId);
    }

    // 2. Vertex Data
    uint vertexIndex = GET_INDEX(pc.indicesAddr, gl_VertexIndex);
    vec3 localPos = GET_VERTEX_POS(pc.vertexPositionsAddr, vertexIndex).position;

    // 4. Fetch Camera and Transform Vertex
    uint cameraIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraIndex);

    SpotLightComponent light = GET_SPOT_LIGHT(pc.spotLightDataAddr, lightDenseIndex);
    gl_Position = camera.viewProjVulkan * light.transform * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outShadowDenseIndex = shadowDenseIndex;
    outCameraIndex = cameraIndex;
}