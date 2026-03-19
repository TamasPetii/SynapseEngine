#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inMeshletColor;
layout(location = 3) in flat uvec4 inIndices; // x: entityId, y: modelIndex, z: meshIndex, w: lodIndex

layout(location = 0) out vec4 outFragColor;

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
    uint padding[3]; 
};

struct ModelComponent { 
    uint entityIndex; 
    uint modelIndex; 
    uint flags; 
    uint materialOffset;  
};

layout(buffer_reference, std430) readonly buffer MaterialBuffer       { GpuMaterial data[]; };
layout(buffer_reference, std430) readonly buffer MaterialLookupBuffer { uint data[]; };
layout(buffer_reference, std430) readonly buffer ModelComponentBuffer { ModelComponent data[]; };
layout(buffer_reference, std430) readonly buffer ModelSparseMap       { uint data[]; };

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
    uint64_t materialLookupBuffer; 
    uint64_t materialBuffer; 
    uint activeCameraEntity;
    uint meshletOffsetStart; 
    uint visualizeMeshlet; 
    float screenWidth;
    float screenHeight;
} pc;

vec4 sampleLoadedTexture2D(uint textureID, uint samplerID, vec2 uv) { 
    return texture(sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), uv); 
}

void main() 
{ 
    ModelSparseMap sparseMap = ModelSparseMap(pc.modelSparseMapBufferAddr); 
    ModelComponentBuffer modelComponents = ModelComponentBuffer(pc.modelBufferAddr); 
    MaterialLookupBuffer materialLookup = MaterialLookupBuffer(pc.materialLookupBuffer); 
    MaterialBuffer globalMaterials = MaterialBuffer(pc.materialBuffer); 

    uint entityId = inIndices.x; 
    ModelComponent comp = modelComponents.data[sparseMap.data[entityId]]; 

    uint flatMaterialIndex = comp.materialOffset + inIndices.z; 
    uint materialId = materialLookup.data[flatMaterialIndex]; 
    GpuMaterial mat = globalMaterials.data[materialId]; 

    /*
    vec4 texColor = sampleLoadedTexture2D(mat.albedoTexture, 0, inUV * mat.uvScale);
    vec3 baseColor = mat.color.rgb * texColor.rgb; 
      
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3)); 
    float ndotl = max(dot(normalize(inNormal), lightDir), 0.1); 
    outFragColor = vec4(baseColor * ndotl, mat.color.a * texColor.a); 
    */

    outFragColor = vec4(mat.color.xyz, 1.0); 
}