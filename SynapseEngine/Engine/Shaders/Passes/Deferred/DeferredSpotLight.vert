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

void main() 
{
    // 1. Resolve Entity ID and Sparse Indexes
    uint entityId = GET_VISIBLE_SPOT_LIGHT(pc.visibleLightAddr, gl_InstanceIndex);
    uint lightDenseIndex = GET_SPARSE_INDEX(pc.spotLightSparseMapAddr, entityId);   
    uint shadowDenseIndex = INVALID_INDEX; 

    if (pc.spotLightShadowSparseMapAddr != 0) {
        shadowDenseIndex = GET_SPARSE_INDEX(pc.spotLightShadowSparseMapAddr, entityId);
    }

    // 2. Fetch Light, Collider and Vertex Data
    SpotLightColliderGPU collider = GET_SPOT_LIGHT_COLLIDER(pc.spotLightColliderDataAddr, lightDenseIndex);
    
    uint vertexIndex = GET_INDEX(pc.indicesAddr, gl_VertexIndex);
    vec3 localPos = GET_VERTEX_POS(pc.vertexPositionsAddr, vertexIndex).position;

    // 3. Build Model Matrix from AABB
    vec3 extents = (collider.aabbMax - collider.aabbMin) * 0.5;
    vec3 center = (collider.aabbMax + collider.aabbMin) * 0.5;

    mat4 model = mat4(1.0);
    model[0][0] = extents.x;
    model[1][1] = extents.y;
    model[2][2] = extents.z;
    model[3] = vec4(center, 1.0);
    
    // 4. Fetch Camera and Transform Vertex
    uint cameraIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraIndex);

    gl_Position = camera.viewProjVulkan * model * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outShadowDenseIndex = shadowDenseIndex;
    outCameraIndex = cameraIndex;
}