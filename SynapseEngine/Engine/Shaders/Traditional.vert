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

struct ModelComponent { 
    uint entityIndex; 
    uint modelIndex; 
    uint flags; 
    uint materialOffset;  
};

struct GpuVertexSkinData {
    uvec4 boneIndices;
    vec4 boneWeights;
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
layout(buffer_reference, std430) readonly buffer ModelSparseMap       { uint data[]; };
layout(buffer_reference, std430) readonly buffer MaterialLookupBuffer { uint data[]; };
layout(buffer_reference, std430) readonly buffer ModelComponentBuffer { ModelComponent data[]; };

layout(buffer_reference, std430) readonly buffer AnimationAddressBuffer { GpuAnimationAddresses data[]; };
layout(buffer_reference, std430) readonly buffer AnimationComponentBuffer { AnimationComponent data[]; };
layout(buffer_reference, std430) readonly buffer VertexSkinDataBuffer { GpuVertexSkinData data[]; };

layout(push_constant) uniform PushConstants {
    uint64_t modelAddressBuffer; 
    uint64_t animationAddressBuffer;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;
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
    uint64_t materialLookupBuffer; 
    uint64_t materialBuffer; 
    uint activeCameraEntity;
    uint baseDescriptorOffset;
    uint materialRenderType;
} pc;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec4 outTangent;
layout(location = 2) out vec2 outUV;
layout(location = 3) out flat uvec4 outId; //(EntityID, MaterialID,  MeshIndex, LodIndex) 

void main() {
    // 1. Descriptor
    DescriptorBuffer descriptors = DescriptorBuffer(pc.globalIndirectCommandDescriptorBuffers);
    MeshDrawDescriptor desc = descriptors.data[pc.baseDescriptorOffset + gl_DrawIDARB];

    // 2. Instance és Entitás
    InstanceBuffer instances = InstanceBuffer(pc.globalInstanceBuffers);
    uint rawEntityData = instances.data[desc.instanceOffset + gl_InstanceIndex];
    uint entityId = rawEntityData & ~(1u << 31);

    // 3. Modell komponens és Anyag
    ModelSparseMap sparseMap = ModelSparseMap(pc.modelSparseMapBufferAddr);
    ModelComponentBuffer modelComponents = ModelComponentBuffer(pc.modelBufferAddr);
    MaterialLookupBuffer materialLookup = MaterialLookupBuffer(pc.materialLookupBuffer);
    
    uint sparseIndex = sparseMap.data[entityId];
    ModelComponent comp = modelComponents.data[sparseIndex];

    // 4. Modell memóriacímek
    ModelAddressBuffer modelAddresses = ModelAddressBuffer(pc.modelAddressBuffer);
    GpuModelAddresses addrs = modelAddresses.data[desc.modelIndex];

    // 5. Nyers Vertex adatok
    IndexBuffer indices = IndexBuffer(addrs.indices);
    uint realVertexIndex = indices.data[gl_VertexIndex];
    
    PositionBuffer positions = PositionBuffer(addrs.vertexPositions);
    AttributeBuffer attributes = AttributeBuffer(addrs.vertexAttributes);
    
    GpuVertexPosition v = positions.data[realVertexIndex];
    GpuVertexAttributes attr = attributes.data[realVertexIndex];

    // 6. Transform és Camera adatok
    SparseMapBuffer transformMap = SparseMapBuffer(pc.transformSparseMapBufferAddr);
    TransformPool transforms = TransformPool(pc.transformBufferAddr);
    uint transformDenseIndex = transformMap.data[entityId];
    TransformComponent transform = transforms.data[transformDenseIndex];

    SparseMapBuffer cameraMap = SparseMapBuffer(pc.cameraSparseMapBufferAddr);
    CameraPool cameras = CameraPool(pc.cameraBufferAddr);
    uint cameraDenseIndex = cameraMap.data[pc.activeCameraEntity];
    CameraComponent camera = cameras.data[cameraDenseIndex];

    // 7. Statikus Modell belső hierarchia (Itt olvassuk ki a defaultot!)
    uint packedIndex = v.packedIndex;
    uint meshIndex = (packedIndex >> 16) & 0xFFFFu;
    uint nodeIndex = packedIndex & 0xFFFFu;
    
    NodeBuffer staticNodes = NodeBuffer(addrs.nodeTransforms);
    GpuNodeTransform staticNodeTransform = staticNodes.data[nodeIndex];

    mat4 finalModelMat = staticNodeTransform.globalTransform;
    mat4 finalModelMatIT = staticNodeTransform.globalTransformIT;

    if(pc.animationSparseMapBufferAddr != 0)
    {
        // 8. Animáció
        SparseMapBuffer animSparseMap = SparseMapBuffer(pc.animationSparseMapBufferAddr);
        uint animSparseIndex = animSparseMap.data[entityId];

        if (animSparseIndex != 0xFFFFFFFFu) 
        {
            AnimationComponentBuffer animComponents = AnimationComponentBuffer(pc.animationBufferAddr);
            AnimationComponent animComp = animComponents.data[animSparseIndex];

            if (animComp.animationIndex != 0xFFFFFFFFu) {
                AnimationAddressBuffer animAddresses = AnimationAddressBuffer(pc.animationAddressBuffer);
                GpuAnimationAddresses animAddrs = animAddresses.data[animComp.animationIndex];

                VertexSkinDataBuffer skinBuffer = VertexSkinDataBuffer(animAddrs.vertexSkinData);
                GpuVertexSkinData skin = skinBuffer.data[realVertexIndex];

                NodeBuffer animNodes = NodeBuffer(animAddrs.nodeTransforms);

                mat4 skinMat = mat4(0.0);
                mat4 skinMatIT = mat4(0.0);
        
                uint frameOffset = animComp.frameIndex * animAddrs.descriptor.nodeCount;
                bool hasValidBone = false;

                for (int i = 0; i < 4; ++i) {
                    float weight = skin.boneWeights[i];
                    if (weight == 0.0) continue; 
            
                    uint boneIdx = skin.boneIndices[i];
                    if (boneIdx != 0xFFFFFFFFu) {
                        GpuNodeTransform boneNode = animNodes.data[frameOffset + boneIdx];
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
    }

    uint flatMaterialIndex = comp.materialOffset + meshIndex;
    uint resolvedMaterialId = materialLookup.data[flatMaterialIndex];

    gl_Position = camera.viewProjVulkan * transform.transform * finalModelMat * vec4(v.position, 1.0);

    vec3 normal = (transform.transformIT * finalModelMatIT * vec4(attr.normal, 0)).xyz;
    vec3 tangent = (transform.transform * finalModelMat * vec4(attr.tangent, 0)).xyz;

    outNormal = normal;
    outTangent = vec4(tangent, 1.0 /*Todo: Invert Normal from model!*/);
    outUV = vec2(attr.uv_x, 1.0 - attr.uv_y);
    outId = uvec4(entityId, resolvedMaterialId, meshIndex, desc.lodIndex);
}