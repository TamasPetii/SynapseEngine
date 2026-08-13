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
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/FrameGlobalContext.glsl"
#include "../../Includes/Common/Camera.glsl"
#include "../../Includes/Common/Mesh.glsl"
#include "../../Includes/Common/Model.glsl"
#include "../../Includes/Common/Transform.glsl"
#include "../../Includes/Common/Animation.glsl"
#include "../../Includes/Utils/ColorMath.glsl"
#include "../../Includes/Common/StaticChunk.glsl"

layout(location = 0) out vec4 outColor;

#include "../../Includes/PushConstants/WireframeMeshPC.glsl"

layout(push_constant) uniform PushConstants {
    WireframeMeshPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Fetch Descriptor and Entity ID
    MeshDrawDescriptor desc = GET_DRAW_DESCRIPTOR(ctx.globalIndirectCommandDescriptorBufferAddr, gl_DrawIDARB);
    uint rawEntityData = GET_INSTANCE(ctx.globalInstanceIndexBufferAddr, desc.instanceOffset + gl_InstanceIndex);
    uint entityId = rawEntityData & ~(1u << 31);
    
    // 2. Fetch Collider
    GpuModelAddresses addrs = GET_MODEL_ADDRESSES(ctx.modelAddressBufferAddr, desc.modelIndex);
    GpuMeshCollider collider = GET_MESH_COLLIDER(addrs.meshColliders, desc.meshIndex);

    // Evaluate Animation Collider if active
    if (ctx.animationSparseMapBufferAddr != 0) {
        uint animIdx = GET_SPARSE_INDEX(ctx.animationSparseMapBufferAddr, entityId);
        if (animIdx != INVALID_INDEX) {
            AnimationComponent animComp = GET_ANIM_COMP(ctx.animationBufferAddr, animIdx);
            if (animComp.animationIndex != INVALID_INDEX) {
                GpuAnimationAddresses animAddrs = GET_ANIM_ADDRESSES(ctx.animationAddressBufferAddr, animComp.animationIndex);

                if (animAddrs.isReady == 1) {
                    uint frameOffset = animComp.frameIndex * animAddrs.descriptor.globalMeshCount;
                    collider = GET_MESH_COLLIDER(animAddrs.frameMeshColliders, frameOffset + desc.meshIndex);
                }
            }
        }
    }

    // 3. Fetch Vertex Data
    uint realVertexIndex = GET_INDEX(pc.indexBufferAddr, gl_VertexIndex);
    GpuVertexPosition v = GET_VERTEX_POS(pc.vertexPositionBufferAddr, realVertexIndex);

    // 4. Calculate Local Position
    vec3 localPos;
    if (pc.shapeType == WIREFRAME_MESH_SHAPE_TYPE_SPHERE) {
        localPos = collider.center + (v.position * collider.radius);
    } 
    else if (pc.shapeType == WIREFRAME_MESH_SHAPE_TYPE_CUBE) {
        vec3 localExtents = (collider.aabbMax - collider.aabbMin) * 0.5;
        vec3 localCenter = (collider.aabbMax + collider.aabbMin) * 0.5;
        localPos = localCenter + (v.position * localExtents); 
    }

    // 5. Apply Transform and Camera
    uint transformDenseIndex = GET_SPARSE_INDEX(ctx.transformSparseMapBufferAddr, entityId);
    TransformComponent transform = GET_TRANSFORM(ctx.transformBufferAddr, transformDenseIndex);
    
    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    gl_Position = camera.viewProjVulkan * transform.transform * vec4(localPos, 1.0);
    outColor = vec4(idToColor(entityId ^ desc.meshIndex ^ desc.lodIndex), 1.0);
}