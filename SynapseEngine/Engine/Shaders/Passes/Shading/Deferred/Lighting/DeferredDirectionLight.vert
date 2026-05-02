#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/DirectionLight.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out flat uint outLightDenseIndex;
layout(location = 2) out flat uint outShadowDenseIndex;
layout(location = 3) out flat uint outCameraIndex;

#include "../../Includes/PushConstants/DeferredDirectionLightPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredDirectionLightPC pc;
};

void main() 
{
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);

    uint entityId = GET_VISIBLE_DIRECTION_LIGHT(pc.visibleLightAddr, gl_InstanceIndex);   
    uint lightDenseIndex = GET_SPARSE_INDEX(pc.directionLightSparseMapAddr, entityId);   
    uint shadowDenseIndex = INVALID_INDEX; 

    if (pc.directionLightShadowSparseMapAddr != 0) {
        shadowDenseIndex = GET_SPARSE_INDEX(pc.directionLightShadowSparseMapAddr, entityId);
    }

    uint cameraIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);

    outLightDenseIndex = lightDenseIndex;
    outShadowDenseIndex = shadowDenseIndex;
    outCameraIndex = cameraIndex;
}