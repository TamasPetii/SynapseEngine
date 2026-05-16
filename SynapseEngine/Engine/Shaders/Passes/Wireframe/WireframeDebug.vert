#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/PointLight.glsl"
#include "../../Includes/Common/SpotLight.glsl"
#include "../../Includes/Common/StaticChunk.glsl"

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/WireframeDebugPC.glsl"

layout(push_constant) uniform PushConstants {
    WireframeDebugPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint realVertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(pc.vertexPositionBufferAddr, realVertexIndex);

    vec3 worldPos = vec3(0.0);
    vec3 lightColor = vec3(1.0);

    //Todo: constexpr 0-5

    // 0: Point Sphere, 1: Point Aabb
    if (pc.lightDrawType <= 1) {
        uint entityId = GET_VISIBLE_POINT_LIGHT(ctx.pointLightVisibleIndexBufferAddr, gl_InstanceIndex);
        uint denseIdx = GET_SPARSE_INDEX(ctx.pointLightSparseMapBufferAddr, entityId);
        PointLightColliderGPU col = GET_POINT_LIGHT_COLLIDER(ctx.pointLightColliderBufferAddr, denseIdx);
        PointLightComponent light = GET_POINT_LIGHT(ctx.pointLightDataBufferAddr, denseIdx);
        
        worldPos = col.center + (v.position * col.radius);
        lightColor = light.color;
    } 
    // Spot Light
    else if (pc.lightDrawType >= 2 && pc.lightDrawType <= 4) {
        uint entityId = GET_VISIBLE_SPOT_LIGHT(ctx.spotLightVisibleIndexBufferAddr, gl_InstanceIndex);
        uint denseIdx = GET_SPARSE_INDEX(ctx.spotLightSparseMapBufferAddr, entityId);
        SpotLightColliderGPU col = GET_SPOT_LIGHT_COLLIDER(ctx.spotLightColliderBufferAddr, denseIdx);
        SpotLightComponent light = GET_SPOT_LIGHT(ctx.spotLightDataBufferAddr, denseIdx);
        
        lightColor = light.color;

        // 2: Spot Sphere
        if (pc.lightDrawType == 2) { 
            worldPos = col.center + (v.position * col.radius);
        } 
        // 3: Spot Aabb Box
        else if(pc.lightDrawType == 3)
        { 
            vec3 extents = (col.aabbMax - col.aabbMin) * 0.5;
            vec3 center = (col.aabbMax + col.aabbMin) * 0.5;
            worldPos = center + (v.position * extents);
        }
        // 4: Spot Cone
        else if(pc.lightDrawType == 4) 
        {
            worldPos = (light.transform * vec4(v.position, 1.0)).xyz;
        }
    }
    else if (pc.lightDrawType == 5) {
        uint rawChunkId = GET_VISIBLE_CHUNK(ctx.staticChunkVisibleIndexBufferAddr, gl_InstanceIndex);
        
        bool chunkFullyInside = (rawChunkId >> 31) != 0;
        uint pureChunkId = rawChunkId & 0x7FFFFFFF;

        StaticChunk chunk = GET_STATIC_CHUNK(ctx.staticChunkDataBufferAddr, pureChunkId);

        vec3 extents = (chunk.maxBounds - chunk.minBounds) * 0.5;
        vec3 center = (chunk.maxBounds + chunk.minBounds) * 0.5;
        worldPos = center + (v.position * extents);

        lightColor = chunkFullyInside ? vec3(0.1, 1.0, 0.1) : vec3(1.0, 0.5, 0.0);
    }

    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    gl_Position = camera.viewProjVulkan * vec4(worldPos, 1.0);
    outColor = vec4(lightColor, 1.0);
}