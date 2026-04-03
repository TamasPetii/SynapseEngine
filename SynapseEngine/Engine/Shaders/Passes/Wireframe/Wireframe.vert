#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/Model.glsl"
#include "../../Includes/Common/Transform.glsl"
#include "../../Includes/Common/Animation.glsl"
#include "../../Includes/Utils/ColorMath.glsl"

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/WireframePC.glsl"

layout(push_constant) uniform PushConstants {
    WireframePC pc;
};

void main() {
    // 1. Fetch Descriptor and Entity ID
    MeshDrawDescriptor desc = GET_DRAW_DESCRIPTOR(pc.globalIndirectCommandDescriptorBuffers, pc.drawIdOffset + gl_DrawIDARB);
    uint rawEntityData = GET_INSTANCE(pc.globalInstanceBuffers, desc.instanceOffset + gl_InstanceIndex);
    uint entityId = rawEntityData & ~(1u << 31);
    
    // 2. Fetch Collider
    GpuModelAddresses addrs = GET_MODEL_ADDRESSES(pc.modelAddressBuffer, desc.modelIndex);
    GpuMeshCollider collider = GET_MESH_COLLIDER(addrs.meshColliders, desc.meshIndex);

    // Evaluate Animation Collider if active
    if (pc.animationSparseMapBufferAddr != 0) {
        uint animIdx = GET_SPARSE_INDEX(pc.animationSparseMapBufferAddr, entityId);
        if (animIdx != INVALID_INDEX) {
            AnimationComponent animComp = GET_ANIM_COMP(pc.animationBufferAddr, animIdx);
            if (animComp.animationIndex != INVALID_INDEX) {
                GpuAnimationAddresses animAddrs = GET_ANIM_ADDRESSES(pc.animationAddressBuffer, animComp.animationIndex);
                uint frameOffset = animComp.frameIndex * animAddrs.descriptor.globalMeshCount;
                collider = GET_MESH_COLLIDER(animAddrs.frameMeshColliders, frameOffset + desc.meshIndex);
            }
        }
    }

    // 3. Fetch Vertex Data
    uint realVertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(pc.vertexBufferAddr, realVertexIndex);

    // 4. Calculate Local Position
    vec3 localPos;
    if (pc.isSphere == 1) {
        localPos = collider.center + (v.position * collider.radius);
    } else {
        vec3 localExtents = (collider.aabbMax - collider.aabbMin) * 0.5;
        vec3 localCenter = (collider.aabbMax + collider.aabbMin) * 0.5;
        localPos = localCenter + (v.position * localExtents); 
    }

    // 5. Apply Transform and Camera
    uint transformDenseIndex = GET_SPARSE_INDEX(pc.transformSparseMapBufferAddr, entityId);
    TransformComponent transform = GET_TRANSFORM(pc.transformBufferAddr, transformDenseIndex);
    
    uint cameraDenseIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraDenseIndex);

    gl_Position = camera.viewProjVulkan * transform.transform * vec4(localPos, 1.0);
    outColor = vec4(idToColor(entityId ^ desc.meshIndex ^ desc.lodIndex), 1.0);
}