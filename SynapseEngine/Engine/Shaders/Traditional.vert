#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : require

struct GpuVertexPosition {
    vec3 position;
    uint packedIndex;
};

struct GpuVertexAttributes {
    vec3 normal;
    float uv_x;
    vec3 tangent;
    float uv_y;
};

struct GpuNodeTransform {
    mat4 globalTransform;
    mat4 globalTransformIT;
};

struct GpuMeshCollider { 
    vec3 center; 
    float radius; 
    vec3 aabbMin; 
    float padding0; 
    vec3 aabbMax; 
    float padding1;
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

struct TransformComponent {
    mat4 transform;
    mat4 transformIT;
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

layout(buffer_reference, std430) readonly buffer PositionBuffer   { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly buffer AttributeBuffer  { GpuVertexAttributes data[]; };
layout(buffer_reference, std430) readonly buffer IndexBuffer      { uint data[]; };
layout(buffer_reference, std430) readonly buffer NodeBuffer       { GpuNodeTransform data[]; };

layout(buffer_reference, std430) readonly buffer ModelAddressBuffer { GpuModelAddresses data[]; };
layout(buffer_reference, std430) readonly buffer DescriptorBuffer   { MeshDrawDescriptor data[]; };
layout(buffer_reference, std430) readonly buffer InstanceBuffer     { uint data[]; };

layout(buffer_reference, std430) readonly buffer SparseMapBuffer    { uint data[]; };
layout(buffer_reference, std430) readonly buffer TransformPool      { TransformComponent data[]; };
layout(buffer_reference, std430) readonly buffer CameraPool         { CameraComponent data[]; };

layout(push_constant) uniform PushConstants {
    uint64_t modelAddressBuffer;

    uint64_t globalDrawCountBuffers;
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandBuffers;
    uint64_t globalIndirectCommandDescriptorBuffers;
    uint64_t globalModelAllocationBuffers;
    uint64_t globalMeshAllocationBuffers;

    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t modelBufferAddr;
    uint64_t modelSparseMapBufferAddr;

    uint activeCameraEntity;
    uint meshletOffsetStart;

    uint padding0;
    uint padding1;
} pc;

layout(location = 0) out vec3 outNormal;
layout(location = 1) flat out uint outLodIndex;

void main() {
    // 1. Melyik Descriptorhoz tartozik ez a draw hívás?
    DescriptorBuffer descriptors = DescriptorBuffer(pc.globalIndirectCommandDescriptorBuffers);
    MeshDrawDescriptor desc = descriptors.data[gl_DrawIDARB];

    // 2. Melyik Entitást rajzoljuk éppen? (Frustum Culling eredménye)
    InstanceBuffer instances = InstanceBuffer(pc.globalInstanceBuffers);
    uint rawEntityData = instances.data[desc.instanceOffset + gl_InstanceIndex];
    uint entityId = rawEntityData & ~(1u << 31);

    // 3. Modell memóriacímeinek feloldása
    ModelAddressBuffer modelAddresses = ModelAddressBuffer(pc.modelAddressBuffer);
    GpuModelAddresses addrs = modelAddresses.data[desc.modelIndex];

    // 4. Vertex adatok kinyerése pointereken keresztül
    IndexBuffer indices = IndexBuffer(addrs.indices);
    uint realVertexIndex = indices.data[gl_VertexIndex];
    
    PositionBuffer positions = PositionBuffer(addrs.vertexPositions);
    AttributeBuffer attributes = AttributeBuffer(addrs.vertexAttributes);
    
    GpuVertexPosition v = positions.data[realVertexIndex];
    GpuVertexAttributes attr = attributes.data[realVertexIndex];

    // 5. Entitás Transform adatainak lekérése
    SparseMapBuffer transformMap = SparseMapBuffer(pc.transformSparseMapBufferAddr);
    TransformPool transforms = TransformPool(pc.transformBufferAddr);
    uint transformDenseIndex = transformMap.data[entityId];
    TransformComponent transform = transforms.data[transformDenseIndex];

    // 6. Kamera adatainak lekérése
    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);
    CameraPool cameras = CameraPool(pc.cameraBufferAddr);
    uint cameraDenseIndex = cameraMap.data[pc.activeCameraEntity];
    CameraComponent camera = cameras.data[cameraDenseIndex];

    // 7. Modell belső hierarchia transzformáció
    uint nodeIndex = v.packedIndex & 0xFFFFu;
    NodeBuffer nodes = NodeBuffer(addrs.nodeTransforms);
    GpuNodeTransform nodeTransform = nodes.data[nodeIndex];

    // 8. Végső mátrixszorzások (Camera * Transform * Node * Vertex)
    gl_Position = camera.viewProjVulkan * transform.transform * nodeTransform.globalTransform * vec4(v.position, 1.0);
   
    // Normálvektor transzformálása
    outNormal = normalize(transform.transformIT * nodeTransform.globalTransformIT * vec4(attr.normal, 0)).xyz;
    outLodIndex = desc.lodIndex;
}