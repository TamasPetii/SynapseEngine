#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/PointLight.glsl"
#include "../../Includes/Common/SpotLight.glsl"

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/WireframeLightPC.glsl"

layout(push_constant) uniform PushConstants {
    WireframeLightPC pc;
};

void main() {
    uint entityId = VisiblePointLightBuffer(pc.visibleLightAddr).data[gl_InstanceIndex];
    uint denseIdx = GET_SPARSE_INDEX(pc.lightSparseMapAddr, entityId);

    uint realVertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(pc.vertexBufferAddr, realVertexIndex);

    vec3 worldPos = vec3(0.0);
    vec3 lightColor = vec3(1.0);

    // 0: Point Sphere, 1: Point Aabb
    if (pc.lightDrawType <= 1) {
        PointLightColliderGPU col = GET_POINT_LIGHT_COLLIDER(pc.lightColliderDataAddr, denseIdx);
        PointLightComponent light = GET_POINT_LIGHT(pc.lightDataAddr, denseIdx);
        
        worldPos = col.center + (v.position * col.radius);
        lightColor = light.color;
    } 
    // Spot Light
    else {
        SpotLightColliderGPU col = GET_SPOT_LIGHT_COLLIDER(pc.lightColliderDataAddr, denseIdx);
        SpotLightComponent light = GET_SPOT_LIGHT(pc.lightDataAddr, denseIdx);
        
        lightColor = light.color;

        // 2: Spot Sphere
        if (pc.lightDrawType == 2) { 
            worldPos = col.center + (v.position * col.radius);
        } 
        // 3: Spot Aabb Box
        else 
        { 
            vec3 extents = (col.aabbMax - col.aabbMin) * 0.5;
            vec3 center = (col.aabbMax + col.aabbMin) * 0.5;
            worldPos = center + (v.position * extents);
        }
    }

    uint cameraDenseIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraDenseIndex);

    gl_Position = camera.viewProjVulkan * vec4(worldPos, 1.0);
    outColor = vec4(lightColor, 1.0);
}