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
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_gpu_shader_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Mesh.glsl"
#include "../../../Includes/Common/Model.glsl"
#include "../../../Includes/Common/Transform.glsl"
#include "../../../Includes/Common/Animation.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/PointLight.glsl"

layout(location = 0) out vec2 outUV;
layout(location = 1) out flat uint outMaterialId;

#include "../../../Includes/PushConstants/PointLightShadowTraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   PointLightShadowTraditionalMeshletPassPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Fetch Point Specific Draw Descriptor
    MeshDrawDescriptor desc = GET_DRAW_DESCRIPTOR(ctx.pointLightDrawDescriptorBufferAddr, pc.baseDescriptorOffset + gl_DrawIDARB);

    // 2. Fetch Instance and Entity ID with Bit-unpacking
    uint shadowInstanceOffset = desc.instanceOffset + gl_InstanceIndex;
    uvec2 payload = GET_POINT_SHADOW_INSTANCE(ctx.pointLightShadowInstanceBufferAddr, shadowInstanceOffset);
    
    uint entityId = payload.x & 0x7FFFFFFF;
    uint faceIndex = payload.y >> 29;
    uint lightIdx = payload.y & 0x1FFFFFFF;

    // 3. Fetch Model Component & Material Lookup
    uint modelDenseIndex = GET_SPARSE_INDEX(ctx.modelSparseMapBufferAddr, entityId);
    uint transformDenseIndex = GET_SPARSE_INDEX(ctx.transformSparseMapBufferAddr, entityId);

    if (transformDenseIndex == INVALID_INDEX || modelDenseIndex == INVALID_INDEX) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        return; 
    }

    ModelComponent comp = GET_MODEL_COMP(ctx.modelBufferAddr, modelDenseIndex);
    
    // 4. Fetch Model Addresses & Raw Vertex Data
    GpuModelAddresses addrs = GET_MODEL_ADDRESSES(ctx.modelAddressBufferAddr, desc.modelIndex);
    uint realVertexIndex = GET_INDEX(addrs.indices, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(addrs.vertexPositions, realVertexIndex);

    // 5. Fetch Transform
    TransformComponent transform = GET_TRANSFORM(ctx.transformBufferAddr, transformDenseIndex);

    // 6. Evaluate Static Hierarchy (Default pose)
    uint nodeIndex = UNPACK_UINT16_X(v.packedIndex);
    uint meshIndex = UNPACK_UINT16_Y(v.packedIndex);
    GpuNodeTransform staticNodeTransform = GET_NODE_TRANSFORM(addrs.nodeTransforms, nodeIndex);
    mat4 finalModelMat = staticNodeTransform.globalTransform;

    // 7. Evaluate Animation & Skinning
    if (ctx.animationSparseMapBufferAddr != 0) {
        uint animSparseIndex = GET_SPARSE_INDEX(ctx.animationSparseMapBufferAddr, entityId);
        if (animSparseIndex != INVALID_INDEX) {
            AnimationComponent animComp = GET_ANIM_COMP(ctx.animationBufferAddr, animSparseIndex);
            if (animComp.animationIndex != INVALID_INDEX) {
                GpuAnimationAddresses animAddrs = GET_ANIM_ADDRESSES(ctx.animationAddressBufferAddr, animComp.animationIndex);

                if (animAddrs.isReady == 1) {
                    GpuVertexSkinData skin = GET_SKIN_DATA(animAddrs.vertexSkinData, realVertexIndex);

                    mat4 skinMat = mat4(0.0);
                    uint frameOffset = animComp.frameIndex * animAddrs.descriptor.nodeCount;
                    bool hasValidBone = false;

                    for (int i = 0; i < 4; ++i) {
                        float weight = skin.boneWeights[i];
                        if (weight == 0.0) continue; 
            
                        uint boneIdx = skin.boneIndices[i];
                        if (boneIdx != INVALID_INDEX) {
                            GpuNodeTransform boneNode = GET_NODE_TRANSFORM(animAddrs.nodeTransforms, frameOffset + boneIdx);
                            skinMat += boneNode.globalTransform * weight;
                            hasValidBone = true;
                        }
                    }

                    if (hasValidBone) {
                        finalModelMat = skinMat;
                    }  
                }
            }    
        }
    }

    // 8. Resolve Point Light Shadow component 
    uint lightEntity = GET_POINT_VISIBLE_SHADOW_LIGHT(ctx.pointLightVisibleShadowIndexBufferAddr, lightIdx);
    uint lightShadowDenseIndex = GET_SPARSE_INDEX(ctx.pointLightShadowSparseMapBufferAddr, lightEntity);
    
    PointLightShadowComponent shadowComp = GET_POINT_LIGHT_SHADOW(ctx.pointLightShadowDataBufferAddr, lightShadowDenseIndex);
    mat4 viewProj = shadowComp.viewProjs[faceIndex];

    // 9. Calculate Final World Position and Outputs
    vec4 clipPos = viewProj * transform.transform * finalModelMat * vec4(v.position, 1.0);

    gl_ClipDistance[0] = clipPos.w + clipPos.x;
    gl_ClipDistance[1] = clipPos.w - clipPos.x;
    gl_ClipDistance[2] = clipPos.w + clipPos.y;
    gl_ClipDistance[3] = clipPos.w - clipPos.y;

    vec4 rect = shadowComp.atlasRects[faceIndex];
    vec2 scale = rect.zw; 
    vec2 offset = rect.xy * 2.0 + rect.zw - 1.0;
    
    // Atlas Positioning
    clipPos.xy = clipPos.xy * scale + offset * clipPos.w;
    gl_Position = clipPos;

    #ifdef ENABLE_ALPHA_TEST

    uint flatMaterialIndex = comp.materialOffset + meshIndex;
    uint resolvedMaterialId = GET_MATERIAL_INDEX(ctx.materialLookupBufferAddr, flatMaterialIndex);
    outMaterialId = resolvedMaterialId;

    GpuVertexAttributes attr = GET_VERTEX_ATTR(addrs.vertexAttributes, realVertexIndex);
    outUV = vec2(attr.uv_x, 1.0 - attr.uv_y);

    #else
    outMaterialId = 0;
    outUV = vec2(0.0);
    #endif
}