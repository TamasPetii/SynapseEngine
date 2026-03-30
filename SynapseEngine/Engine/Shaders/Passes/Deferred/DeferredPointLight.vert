#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/PointLight.glsl"

layout(location = 0) out flat uint outLightDenseIndex;
layout(location = 1) out flat uint outShadowDenseIndex;
layout(location = 2) out flat uint outCameraIndex;

#include "../../Includes/PushConstants/DeferredPointLightPC.glsl"

void main() 
{
    // 1. Resolve Entity ID and Sparse Indexes
    uint entityId = GET_VISIBLE_LIGHT(pc.visibleLightAddr, gl_InstanceIndex);   
    uint lightDenseIndex = GET_SPARSE_INDEX(pc.pointLightSparseMapAddr, entityId);   
    uint shadowDenseIndex = INVALID_INDEX; 

    if (pc.pointLightShadowSparseMapAddr != 0) {
        shadowDenseIndex = GET_SPARSE_INDEX(pc.pointLightShadowSparseMapAddr, entityId);
    }

    // 2. Fetch Light and Vertex Data
    PointLightComponent light = GET_POINT_LIGHT(pc.pointLightDataAddr, lightDenseIndex);
    uint vertexIndex = GET_INDEX(pc.indicesAddr, gl_VertexIndex);
    vec3 localPos = GET_VERTEX_POS(pc.vertexPositionsAddr, vertexIndex).position;

    // 3. Build Model Matrix
    mat4 model = mat4(1.0);
    model[0][0] = light.radius;
    model[1][1] = light.radius;
    model[2][2] = light.radius;
    model[3] = vec4(light.position, 1.0);
    
    // 4. Fetch Camera and Transform Vertex
    uint cameraIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraIndex);

    gl_Position = camera.viewProjVulkan * model * vec4(localPos, 1.0);
    
    outLightDenseIndex = lightDenseIndex;
    outShadowDenseIndex = shadowDenseIndex;
    outCameraIndex = cameraIndex;
}