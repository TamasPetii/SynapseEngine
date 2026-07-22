#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Debug.glsl"
#include "../../Includes/Common/Camera.glsl"

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/PhysicsDebugPC.glsl"

layout(push_constant) uniform PushConstants {
    PhysicsDebugPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    uint trueVertexIndex = GET_PHYSICS_DEBUG_INDEX(pc.joltDebugIndexBufferAddr, gl_VertexIndex);
    PhysicsDebugVertex v = GET_PHYSICS_DEBUG_VERTEX(pc.joltDebugVertexBufferAddr, trueVertexIndex);

    PhysicsDebugInstance inst = GET_PHYSICS_DEBUG_INSTANCE(pc.joltDebugInstanceBufferAddr, gl_InstanceIndex);

    float r = float(inst.color & 0xFF) / 255.0;
    float g = float((inst.color >> 8) & 0xFF) / 255.0;
    float b = float((inst.color >> 16) & 0xFF) / 255.0;
    float a = float((inst.color >> 24) & 0xFF) / 255.0;

    vec4 worldPos = inst.transform * v.position;
    gl_Position = camera.viewProjVulkan * worldPos;

    outColor = vec4(r, g, b, a);
}