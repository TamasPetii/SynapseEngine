#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; //(EntityID, MaterialID, MeshIndex, LodIndex) 

layout(location = 0) out uint outEntityId;

layout(set = 0, binding = 0) uniform sampler globalSamplers[];
layout(set = 0, binding = 1) uniform texture2D bindlessTextures[];

struct GpuMaterial { 
    vec4 color; 
    vec3 emissiveColor; 
    float emissiveIntensity; 
    vec2 uvScale; 
    float metalness; 
    float roughness;
    float aoStrength; 
    uint packedFlags; 
    uint albedoTexture; 
    uint normalTexture; 
    uint metalnessTexture; 
    uint roughnessTexture; 
    uint metallicRoughnessTexture; 
    uint emissiveTexture; 
    uint ambientOcclusionTexture; 
    uint padding0;
    uint padding1; 
    uint padding2; 
};

layout(buffer_reference, std430) readonly buffer MaterialBuffer { GpuMaterial data[]; };

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

vec4 sampleLoadedTexture2D(uint textureID, uint samplerID, vec2 uv) { 
    return texture(sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), uv);
}

void main() 
{ 
    uint entityId = inId.x;
    uint materialId = inId.y;
    uint meshIndex = inId.z;
    uint lodIndex = inId.w;

    MaterialBuffer globalMaterials = MaterialBuffer(pc.materialBuffer); 
    GpuMaterial mat = globalMaterials.data[materialId];

    vec4 albedoTex = sampleLoadedTexture2D(mat.albedoTexture, 0, inUV * mat.uvScale);
    float alpha = mat.color.a * albedoTex.a;

    if (alpha < 0.05) {
        discard;
    }

    outEntityId = entityId;
}