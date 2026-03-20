#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : require

struct GpuVertexPosition {
    vec3 position;
    uint packedIndex;
};

struct GpuMeshCollider { 
    vec3 center; 
    float radius; 
    vec3 aabbMin; 
    float padding0; 
    vec3 aabbMax; 
    float padding1; 
};

struct GpuNodeTransform {
    mat4 globalTransform;
    mat4 globalTransformIT;
};

struct GpuModelAddresses { 
    uint64_t vertexPositions; 
    uint64_t vertexAttributes; 
    uint64_t indices; 
    uint64_t meshDescriptors; 
    uint64_t meshColliders; 
    uint64_t lodDescriptors; 
    uint64_t meshletVertexIndices; 
    uint64_t meshletTriangleIndices; 
    uint64_t meshletDescriptors; 
    uint64_t meshletDrawDescriptors; 
    uint64_t meshletColliders; 
    uint64_t nodeTransforms; 
    GpuMeshCollider globalCollider; 
    uint vertexCount; 
    uint indexCount; 
    uint averageLodIndexCount; 
    uint meshCount; 
};

struct MeshDrawDescriptor {
    uint modelIndex;
    uint meshIndex;
    uint lodIndex;
    uint instanceOffset;
    uint maxInstances;
    uint indirectIndex;
    uint isMeshletPipeline;
    uint padding;
};

struct CameraComponent { 
    mat4 view; 
    mat4 viewInv; 
    mat4 proj; 
    mat4 projInv; 
    mat4 projVulkan; 
    mat4 projVulkanInv; 
    mat4 viewProj; 
    mat4 viewProjInv; 
    mat4 viewProjVulkan; 
    mat4 viewProjVulkanInv; 
    vec4 eye; 
    vec4 params; 
    vec4 frustum[6]; 
};

struct TransformComponent { 
    mat4 transform; 
    mat4 transformIT; 
};

layout(buffer_reference, std430) readonly buffer PositionBuffer     { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly buffer IndexBuffer        { uint data[]; };
layout(buffer_reference, std430) readonly buffer InstanceBuffer     { uint data[]; };
layout(buffer_reference, std430) readonly buffer DescriptorBuffer   { MeshDrawDescriptor data[]; };
layout(buffer_reference, std430) readonly buffer ModelAddressBuffer { GpuModelAddresses data[]; };
layout(buffer_reference, std430) readonly buffer MeshColliderPool   { GpuMeshCollider data[]; };
layout(buffer_reference, std430) readonly buffer TransformPool      { TransformComponent data[]; };
layout(buffer_reference, std430) readonly buffer CameraPool         { CameraComponent data[]; };
layout(buffer_reference, std430) readonly buffer SparseMapBuffer    { uint data[]; };
layout(buffer_reference, std430) readonly buffer NodeBuffer         { GpuNodeTransform data[]; }; 

layout(push_constant) uniform PushConstants {
    uint64_t modelAddressBuffer;
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandDescriptorBuffers;
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t vertexBufferAddr;
    uint activeCameraEntity;
    uint isSphere;
    vec4 debugColor;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
    DescriptorBuffer descriptors = DescriptorBuffer(pc.globalIndirectCommandDescriptorBuffers);
    MeshDrawDescriptor desc = descriptors.data[gl_DrawIDARB];

    InstanceBuffer instances = InstanceBuffer(pc.globalInstanceBuffers);
    uint rawEntityData = instances.data[desc.instanceOffset + gl_InstanceIndex];
    uint entityId = rawEntityData & ~(1u << 31);

    IndexBuffer indices = IndexBuffer(pc.indexBufferAddr);
    PositionBuffer positions = PositionBuffer(pc.vertexBufferAddr);
    
    uint realVertexIndex = indices.data[gl_VertexIndex];
    GpuVertexPosition v = positions.data[realVertexIndex];

    ModelAddressBuffer addresses = ModelAddressBuffer(pc.modelAddressBuffer);
    GpuModelAddresses addrs = addresses.data[desc.modelIndex];

    MeshColliderPool colliders = MeshColliderPool(addrs.meshColliders);
    GpuMeshCollider collider = colliders.data[desc.meshIndex];

    vec3 localPos;
    if (pc.isSphere == 1) {
        localPos = collider.center + (v.position * collider.radius);
    } else {
        vec3 localExtents = (collider.aabbMax - collider.aabbMin) * 0.5;
        vec3 localCenter = (collider.aabbMax + collider.aabbMin) * 0.5;
        localPos = localCenter + (v.position * localExtents); 
    }

    SparseMapBuffer transformMap = SparseMapBuffer(pc.transformSparseMapBufferAddr);
    TransformPool transforms = TransformPool(pc.transformBufferAddr);
    uint transformDenseIndex = transformMap.data[entityId];
    TransformComponent transform = transforms.data[transformDenseIndex];

    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);
    CameraPool cameras = CameraPool(pc.cameraBufferAddr);
    uint cameraDenseIndex = cameraMap.data[pc.activeCameraEntity];
    CameraComponent camera = cameras.data[cameraDenseIndex];

    uint packedIndex = v.packedIndex;
    uint meshIndex = (packedIndex >> 16) & 0xFFFFu;
    uint nodeIndex = packedIndex & 0xFFFFu;
    NodeBuffer nodes = NodeBuffer(addrs.nodeTransforms);
    GpuNodeTransform nodeTransform = nodes.data[nodeIndex];

    gl_Position = camera.viewProjVulkan * transform.transform * nodeTransform.globalTransform * vec4(localPos, 1.0);
    outColor = pc.debugColor;
}