#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : require

struct GpuVertexPosition 
{
    vec3 position;
    uint packedIndex;
};

struct GpuMeshCollider 
{
    vec3 center;
    float radius;
    vec3 aabbMin;
    float padding0;
    vec3 aabbMax;
    float padding1;
};

struct GpuMeshletCollider 
{
    vec3 center;
    float radius;
    vec3 aabbMin;
    float padding0;
    vec3 aabbMax;
    float padding1;
    vec3 apex;
    float cutoff;
    vec3 axis;
    float padding2;
};

struct CameraComponent 
{
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

struct TransformComponent 
{
    mat4 transform;
    mat4 transformIT;
};

struct GpuModelAddresses 
{
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

struct ModelComponent 
{
    uint entityIndex;
    uint modelIndex;
    uint flags;
    uint materialOffset;
};

struct DebugMeshletInstance 
{
    uint entityId;
    uint globalMeshletIdx;
};

struct AnimationComponent {
    uint animationIndex;
    uint frameIndex;
    uint padding0;
    uint padding1;
};

struct GpuAnimationDescriptor {
    uint frameCount;
    uint nodeCount;
    uint globalVertexCount;
    uint globalMeshCount;
    uint globalMeshletCount;
    float durationInSeconds;
    float sampleRate;
    float padding;
};

struct GpuAnimationAddresses {
    uint64_t vertexSkinData;
    uint64_t nodeTransforms;
    uint64_t frameGlobalColliders;
    uint64_t frameMeshColliders;
    uint64_t frameMeshletColliders;
    uint64_t padding;
    GpuAnimationDescriptor descriptor;
};

layout(buffer_reference, std430) readonly buffer ModelAddressBuffer { GpuModelAddresses data[]; };
layout(buffer_reference, std430) readonly buffer InstanceBuffer { uint data[]; };
layout(buffer_reference, std430) readonly buffer TransformPool { TransformComponent data[]; };
layout(buffer_reference, std430) readonly buffer CameraPool { CameraComponent data[]; };
layout(buffer_reference, std430) readonly buffer MeshletColliderBuffer { GpuMeshletCollider data[]; };
layout(buffer_reference, std430) readonly buffer SparseMapBuffer { uint data[]; };
layout(buffer_reference, std430) readonly buffer PositionBuffer { GpuVertexPosition data[]; };
layout(buffer_reference, std430) readonly buffer IndexBuffer { uint data[]; };
layout(buffer_reference, std430) readonly buffer DebugInstanceBuffer { DebugMeshletInstance instances[]; };
layout(buffer_reference, std430) readonly buffer ModelSparseMap { uint data[]; };
layout(buffer_reference, std430) readonly buffer ModelComponentBuffer { ModelComponent data[]; };
layout(buffer_reference, std430) readonly buffer AnimationComponentBuffer { AnimationComponent data[]; };
layout(buffer_reference, std430) readonly buffer AnimationAddressBuffer { GpuAnimationAddresses data[]; };
layout(buffer_reference, std430) readonly buffer FrameMeshletColliderBuffer { GpuMeshletCollider data[]; };

layout(push_constant) uniform PushConstants 
{
    uint64_t animationAddressBuffer;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;
    uint64_t modelAddressBuffer;
    uint64_t globalInstanceBuffers;
    uint64_t globalIndirectCommandDescriptorBuffers;
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;
    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t indexBufferAddr;
    uint64_t vertexBufferAddr;
    uint64_t debugInstanceBufferAddr;
    uint64_t modelBufferAddr;
    uint64_t modelSparseMapBufferAddr;
    uint activeCameraEntity;
    uint isSphere;
    vec4 debugColor;
} pc;

layout(location = 0) out vec4 outColor;

vec3 generateWireframeColor(uint id) 
{
    uint hash = (id ^ 61) ^ (id >> 16);
    hash = hash + (hash << 3);
    hash = hash ^ (hash >> 4);
    hash = hash * 0x27d4eb2d;
    hash = hash ^ (hash >> 15);
    
    float r = float((hash & 0xFF0000) >> 16) / 255.0;
    float g = float((hash & 0x00FF00) >> 8) / 255.0;
    float b = float(hash & 0x0000FF) / 255.0;
    
    return vec3(r, g, b);
}

void main() 
{
    DebugInstanceBuffer instBuf = DebugInstanceBuffer(pc.debugInstanceBufferAddr);
    DebugMeshletInstance instanceData = instBuf.instances[gl_InstanceIndex];
    
    ModelSparseMap sparseMap = ModelSparseMap(pc.modelSparseMapBufferAddr);
    ModelComponentBuffer modelComponents = ModelComponentBuffer(pc.modelBufferAddr);
    uint sparseIndex = sparseMap.data[instanceData.entityId];
    ModelComponent comp = modelComponents.data[sparseIndex];
    
    ModelAddressBuffer modelAddresses = ModelAddressBuffer(pc.modelAddressBuffer);
    GpuModelAddresses addrs = modelAddresses.data[comp.modelIndex];
    
    MeshletColliderBuffer colliders = MeshletColliderBuffer(addrs.meshletColliders);

    //Static Mesh Collider
    GpuMeshletCollider collider = colliders.data[instanceData.globalMeshletIdx];

    SparseMapBuffer animSparseMap = SparseMapBuffer(pc.animationSparseMapBufferAddr);
    uint animSparseIndex = animSparseMap.data[instanceData.entityId];

    if (animSparseIndex != 0xFFFFFFFFu) {
        AnimationComponentBuffer animComponents = AnimationComponentBuffer(pc.animationBufferAddr);
        AnimationComponent animComp = animComponents.data[animSparseIndex];

        if (animComp.animationIndex != 0xFFFFFFFFu) {
            AnimationAddressBuffer animAddresses = AnimationAddressBuffer(pc.animationAddressBuffer);
            GpuAnimationAddresses animAddrs = animAddresses.data[animComp.animationIndex];

            FrameMeshletColliderBuffer animColliders = FrameMeshletColliderBuffer(animAddrs.frameMeshletColliders);
            uint frameOffset = animComp.frameIndex * animAddrs.descriptor.globalMeshletCount;
            collider = animColliders.data[frameOffset + instanceData.globalMeshletIdx];
        }
    }

    IndexBuffer indices = IndexBuffer(pc.indexBufferAddr);
    PositionBuffer positions = PositionBuffer(pc.vertexBufferAddr);

    uint realVertexIndex = indices.data[gl_VertexIndex];
    GpuVertexPosition v = positions.data[realVertexIndex];

    vec3 localPos;
    if (pc.isSphere == 1) 
    {
        localPos = collider.center + (v.position * collider.radius);
    } 
    else 
    {
        vec3 localExtents = (collider.aabbMax - collider.aabbMin) * 0.5;
        vec3 localCenter = (collider.aabbMax + collider.aabbMin) * 0.5;
        localPos = localCenter + (v.position * localExtents);
    }

    SparseMapBuffer transformMap = SparseMapBuffer(pc.transformSparseMapBufferAddr);
    TransformPool transforms = TransformPool(pc.transformBufferAddr);
    TransformComponent transform = transforms.data[transformMap.data[instanceData.entityId]];

    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);
    CameraPool cameras = CameraPool(pc.cameraBufferAddr);
    CameraComponent camera = cameras.data[cameraMap.data[pc.activeCameraEntity]];

    gl_Position = camera.viewProjVulkan * transform.transform * vec4(localPos, 1.0);  
    outColor = vec4(generateWireframeColor(instanceData.globalMeshletIdx), 1.0);
}