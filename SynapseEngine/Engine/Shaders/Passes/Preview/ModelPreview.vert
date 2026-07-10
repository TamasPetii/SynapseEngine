#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/Model.glsl"
#include "../../Includes/Common/Transform.glsl"

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec4 outTangent;
layout(location = 2) out vec2 outUV;
layout(location = 3) out flat uint outMaterialId;

#include "../../Includes/PushConstants/ModelPreviewPC.glsl"

layout(push_constant) uniform PushConstants {
    ModelPreviewPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    GpuModelAddresses addrs = GET_MODEL_ADDRESSES(ctx.modelAddressBufferAddr, pc.modelId);

    uint realVertexIndex = GET_INDEX(addrs.indices, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(addrs.vertexPositions, realVertexIndex);
    GpuVertexAttributes attr = GET_VERTEX_ATTR(addrs.vertexAttributes, realVertexIndex);

    uint nodeIndex = UNPACK_UINT16_X(v.packedIndex);
    uint meshIndex = UNPACK_UINT16_Y(v.packedIndex);

    GpuNodeTransform staticNodeTransform = GET_NODE_TRANSFORM(addrs.nodeTransforms, nodeIndex);
    mat4 finalModelMat = staticNodeTransform.globalTransform;
    mat4 finalModelMatIT = staticNodeTransform.globalTransformIT;

    gl_Position = pc.mvp * finalModelMat * vec4(v.position, 1.0);
    
    outNormal = (finalModelMatIT * vec4(attr.normal, 0.0)).xyz;
    outTangent = vec4((finalModelMat * vec4(attr.tangent, 0.0)).xyz, 1.0);
    outUV = vec2(attr.uv_x, 1.0 - attr.uv_y);
    outMaterialId = GET_DEFUALT_MATERIAL_INDEX(addrs.meshMaterialIndices, pc.meshIndex);
}