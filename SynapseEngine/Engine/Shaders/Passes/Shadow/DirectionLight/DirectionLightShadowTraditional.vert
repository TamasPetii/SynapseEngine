#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_ARB_shader_draw_parameters : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Mesh.glsl"
#include "../../../Includes/Common/Model.glsl"
#include "../../../Includes/Common/Transform.glsl"
#include "../../../Includes/Common/Animation.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/DirectionLight.glsl"

#include "../../../Includes/PushConstants/DirectionLightShadowTraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   DirectionLightShadowTraditionalMeshletPassPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Fetch Draw Descriptor
    MeshDrawDescriptor desc = GET_DRAW_DESCRIPTOR(ctx.globalIndirectCommandDescriptorBufferAddr, pc.baseDescriptorOffset + gl_DrawIDARB);

    // 2. Fetch Instance and Entity ID
    uint shadowInstanceOffset = (desc.instanceOffset * ctx.directionLightShadowMultiplier) + gl_InstanceIndex;
    uint payload = GET_INSTANCE(ctx.directionLightShadowInstanceBufferAddr, shadowInstanceOffset);

    uint entityId   = payload & 0x3FFFFFFu;
    uint cascadeIdx = (payload >> 26) & 0x3u;
    uint lightIdx   = (payload >> 28) & 0x7u;

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

    // 5. Fetch Transform and Camera
    TransformComponent transform = GET_TRANSFORM(ctx.transformBufferAddr, transformDenseIndex);

    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

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

                    // Accumulate bone weights
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

    // 8. Resolve Directional Light Shadow component 
    uint lightEntity = GET_DIRECTION_VISIBLE_SHADOW_LIGHT(ctx.directionLightVisibleShadowIndexBufferAddr, lightIdx);
    uint lightShadowDenseIndex = GET_SPARSE_INDEX(ctx.directionLightShadowSparseMapBufferAddr, lightEntity);
    mat4 viewProj = GET_DIRECTION_LIGHT_SHADOW(ctx.directionLightShadowDataBufferAddr, lightShadowDenseIndex).cascadeViewProjsVulkan[cascadeIdx];

    // 9. Calculate Final World Position and Outputs
    vec4 clipPos = viewProj * transform.transform * finalModelMat * vec4(v.position, 1.0);

    gl_ClipDistance[0] = clipPos.w + clipPos.x;
    gl_ClipDistance[1] = clipPos.w - clipPos.x;
    gl_ClipDistance[2] = clipPos.w + clipPos.y;
    gl_ClipDistance[3] = clipPos.w - clipPos.y;

    vec4 rect = GET_DIRECTION_LIGHT_SHADOW(ctx.directionLightShadowDataBufferAddr, lightShadowDenseIndex).cascadeAtlasRects[cascadeIdx];
    vec2 scale = rect.zw; 
    vec2 offset = rect.xy * 2.0 + rect.zw - 1.0; 

    clipPos.xy = clipPos.xy * scale + offset * clipPos.w;

    // Atlas Positioning
    gl_Position = clipPos;
}