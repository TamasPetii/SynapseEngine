#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; //(EntityID, MaterialID, MeshletIndex, LodIndex) 

layout(location = 0) out uint outEntityId;
layout(location = 1) out vec4 outDebugTopologyPipeline;
layout(location = 2) out vec4 outDebugMeshletLod;
layout(location = 3) out vec4 outDebugMaterialUv;

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

    uint64_t debugInstanceBufferAddr;
    uint64_t debugAabbIndirectAddr;
    uint64_t debugSphereIndirectAddr;

    uint activeCameraEntity;
    uint baseDescriptorOffset;
    uint disableConeCulling;
    uint materialRenderType;

    float screenWidth;
    float screenHeight;
} pc;

vec4 sampleLoadedTexture2D(uint textureID, uint samplerID, vec2 uv) { 
    return texture(sampler2D(bindlessTextures[nonuniformEXT(textureID)], globalSamplers[nonuniformEXT(samplerID)]), uv);
}

const vec3 LOD_COLORS[4] = vec3[](
    vec3(0.1, 0.9, 0.2), // LOD 0 -> Green
    vec3(0.9, 0.9, 0.1), // LOD 1 -> Yellow
    vec3(0.9, 0.5, 0.1), // LOD 2 -> Orange
    vec3(0.9, 0.1, 0.1)  // LOD 3 -> Red
);

const vec2 MAT_TYPE_COLORS[4] = vec2[](
        vec2(1.0, 0.0), // 0: Opaque 1-Sided      -> Red
        vec2(0.0, 1.0), // 1: Opaque 2-Sided      -> Green
        vec2(1.0, 1.0), // 2: Transparent 1-Sided -> Yellow
        vec2(1.0, 0.4)  // 3: Transparent 2-Sided -> Orange
);

// PCG Hash
uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

vec3 idToColor(uint id) {
    uint h = hash(id);
    return vec3(
        float((h >> 16) & 0xFF) / 255.0,
        float((h >> 8) & 0xFF) / 255.0,
        float(h & 0xFF) / 255.0
    );
}

vec3 getMeshletLodColor(uint entityId, uint meshletIndex, uint lodIndex) {
    vec3 baseLODColor = LOD_COLORS[lodIndex % 4]; 
    
    uint combinedHash = hash(entityId ^ hash(meshletIndex));
    
    vec3 meshletColor = vec3(
        float((combinedHash >> 16) & 0xFF) / 255.0,
        float((combinedHash >> 8) & 0xFF) / 255.0,
        float(combinedHash & 0xFF) / 255.0
    );
    
    vec3 finalColor = mix(baseLODColor, meshletColor, 0.4);
    float randomBrightness = float((combinedHash >> 24) & 0xFF) / 255.0;
    float brightnessScale = 0.3 + (randomBrightness * 1.2);
    return finalColor * brightnessScale;
}

void main() 
{ 
    uint entityId = inId.x;
    uint materialId = inId.y;
    uint meshletIndex = inId.z;
    uint lodIndex = inId.w;

    MaterialBuffer globalMaterials = MaterialBuffer(pc.materialBuffer); 
    GpuMaterial mat = globalMaterials.data[materialId];

    vec2 finalUV = inUV * mat.uvScale;

    // Albedo & Alpha
    vec4 finalColor = mat.color;
    if(mat.albedoTexture != 0xFFFFFFFFu) {
        finalColor *= sampleLoadedTexture2D(mat.albedoTexture, 0, finalUV);
    }

    if (finalColor.a < 0.05) {
        discard;
    }

    outEntityId = entityId;

    vec3 lodHeatmapColor = getMeshletLodColor(entityId, meshletIndex, lodIndex);
    outDebugTopologyPipeline = vec4(lodHeatmapColor, 1.0);

    vec3 pureMeshletColor = idToColor(entityId ^ hash(meshletIndex));
    float pureBrightness = 0.5 + (float((hash(entityId ^ meshletIndex) >> 24) & 0xFF) / 255.0) * 0.5;
    float grayscaleLOD = 1.0 - (float(lodIndex) / 3.0);
    outDebugMeshletLod = vec4(pureMeshletColor * pureBrightness, grayscaleLOD);

    vec2 baseMatColor = MAT_TYPE_COLORS[pc.materialRenderType % 4];
    float idHashModifier = (float(hash(materialId) & 0xFF) / 255.0) * 0.4 - 0.2;
    vec2 finalMatColor = clamp(baseMatColor + idHashModifier, 0.0, 1.0);
    vec2 debugUV = fract(finalUV);
    outDebugMaterialUv = vec4(finalMatColor.x, finalMatColor.y, debugUV.x, debugUV.y);
}