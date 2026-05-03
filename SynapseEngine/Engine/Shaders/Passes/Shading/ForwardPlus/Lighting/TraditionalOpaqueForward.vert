#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/Mesh.glsl"
#include "../../../../Includes/Common/Model.glsl"
#include "../../../../Includes/Common/Transform.glsl"
#include "../../../../Includes/Common/Animation.glsl"

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec4 outTangent;
layout(location = 2) out vec2 outUV;
layout(location = 3) out flat uvec4 outId; // (EntityID, MaterialID, MeshIndex, LodIndex)

#include "../../../../Includes/PushConstants/TraditionalForwardPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalForwardPC pc;
};

void main() {
    MeshDrawDescriptor desc = GET_DRAW_DESCRIPTOR(pc.globalIndirectCommandDescriptorBuffers, pc.baseDescriptorOffset + gl_DrawIDARB);
    uint rawEntityData = GET_INSTANCE(pc.globalInstanceBuffers, desc.instanceOffset + gl_InstanceIndex);
    uint entityId = rawEntityData & ~(1u << 31);

    uint modelDenseIndex = GET_SPARSE_INDEX(pc.modelSparseMapBufferAddr, entityId);
    ModelComponent comp = GET_MODEL_COMP(pc.modelBufferAddr, modelDenseIndex);
    
    GpuModelAddresses addrs = GET_MODEL_ADDRESSES(pc.modelAddressBuffer, desc.modelIndex);
    uint realVertexIndex = GET_INDEX(addrs.indices, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(addrs.vertexPositions, realVertexIndex);
    GpuVertexAttributes attr = GET_VERTEX_ATTR(addrs.vertexAttributes, realVertexIndex);

    uint transformDenseIndex = GET_SPARSE_INDEX(pc.transformSparseMapBufferAddr, entityId);
    TransformComponent transform = GET_TRANSFORM(pc.transformBufferAddr, transformDenseIndex);
    uint cameraDenseIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraDenseIndex);

    uint nodeIndex = UNPACK_UINT16_X(v.packedIndex);
    uint meshIndex = UNPACK_UINT16_Y(v.packedIndex);
    GpuNodeTransform staticNodeTransform = GET_NODE_TRANSFORM(addrs.nodeTransforms, nodeIndex);
    mat4 finalModelMat = staticNodeTransform.globalTransform;
    mat4 finalModelMatIT = staticNodeTransform.globalTransformIT;

    if (pc.animationSparseMapBufferAddr != 0) {
        uint animSparseIndex = GET_SPARSE_INDEX(pc.animationSparseMapBufferAddr, entityId);
        if (animSparseIndex != INVALID_INDEX) {
            AnimationComponent animComp = GET_ANIM_COMP(pc.animationBufferAddr, animSparseIndex);
            if (animComp.animationIndex != INVALID_INDEX) {
                GpuAnimationAddresses animAddrs = GET_ANIM_ADDRESSES(pc.animationAddressBuffer, animComp.animationIndex);
                GpuVertexSkinData skin = GET_SKIN_DATA(animAddrs.vertexSkinData, realVertexIndex);
                mat4 skinMat = mat4(0.0); mat4 skinMatIT = mat4(0.0);
                uint frameOffset = animComp.frameIndex * animAddrs.descriptor.nodeCount;
                bool hasValidBone = false;
                for (int i = 0; i < 4; ++i) {
                    if (skin.boneWeights[i] == 0.0) continue; 
                    uint boneIdx = skin.boneIndices[i];
                    if (boneIdx != INVALID_INDEX) {
                        GpuNodeTransform boneNode = GET_NODE_TRANSFORM(animAddrs.nodeTransforms, frameOffset + boneIdx);
                        skinMat += boneNode.globalTransform * skin.boneWeights[i];
                        skinMatIT += boneNode.globalTransformIT * skin.boneWeights[i];
                        hasValidBone = true;
                    }
                }
                if (hasValidBone) { finalModelMat = skinMat; finalModelMatIT = skinMatIT; }
            }    
        }
    }

    uint flatMaterialIndex = comp.materialOffset + meshIndex;
    uint resolvedMaterialId = GET_MATERIAL_INDEX(pc.materialLookupBuffer, flatMaterialIndex);

    gl_Position = camera.viewProjVulkan * transform.transform * finalModelMat * vec4(v.position, 1.0);
    outNormal = (transform.transformIT * finalModelMatIT * vec4(attr.normal, 0.0)).xyz;
    outTangent = vec4((transform.transform * finalModelMat * vec4(attr.tangent, 0.0)).xyz, 1.0);
    outUV = vec2(attr.uv_x, 1.0 - attr.uv_y);
    outId = uvec4(entityId, resolvedMaterialId, meshIndex, desc.lodIndex);
}