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
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/Model.glsl"
#include "../../Includes/Common/Transform.glsl"
#include "../../Includes/Common/Animation.glsl"

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec4 outTangent;
layout(location = 2) out vec2 outUV;
layout(location = 3) out flat uint outMaterialId;

#include "../../Includes/PushConstants/AnimationPreviewPC.glsl"

layout(push_constant) uniform PushConstants {
    AnimationPreviewPC pc;
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

    if (pc.animationId != INVALID_INDEX && ctx.animationAddressBufferAddr != 0) {
        GpuAnimationAddresses animAddrs = GET_ANIM_ADDRESSES(ctx.animationAddressBufferAddr, pc.animationId);

        if (animAddrs.isReady == 1) {
            GpuVertexSkinData skin = GET_SKIN_DATA(animAddrs.vertexSkinData, realVertexIndex);

            mat4 skinMat = mat4(0.0);
            mat4 skinMatIT = mat4(0.0);
            uint frameOffset = pc.frameIndex * animAddrs.descriptor.nodeCount;
            bool hasValidBone = false;

            for (int i = 0; i < 4; ++i) {
                float weight = skin.boneWeights[i];
                if (weight == 0.0) continue; 
    
                uint boneIdx = skin.boneIndices[i];
                if (boneIdx != INVALID_INDEX) {
                    GpuNodeTransform boneNode = GET_NODE_TRANSFORM(animAddrs.nodeTransforms, frameOffset + boneIdx);
                    skinMat += boneNode.globalTransform * weight;
                    skinMatIT += boneNode.globalTransformIT * weight;
                    hasValidBone = true;
                }
            }

            if (hasValidBone) {
                finalModelMat = skinMat;
                finalModelMatIT = skinMatIT;
            }  
        }
    }

    gl_Position = pc.mvp * finalModelMat * vec4(v.position, 1.0);
    
    outNormal = (finalModelMatIT * vec4(attr.normal, 0.0)).xyz;
    outTangent = vec4((finalModelMat * vec4(attr.tangent, 0.0)).xyz, 1.0);
    outUV = vec2(attr.uv_x, 1.0 - attr.uv_y);
    outMaterialId = GET_DEFUALT_MATERIAL_INDEX(addrs.meshMaterialIndices, pc.meshIndex);
}