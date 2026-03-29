#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; //(EntityID, MaterialID, MeshletID/MeshID, LodID) 

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out uint outEntityId;

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

layout(buffer_reference, std430) readonly buffer MaterialBuffer       { GpuMaterial data[]; };

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

vec3 getDebugColor(uint entityId, uint meshIndex, uint lodIndex, vec2 fragCoord) {
    vec3 baseLODColor = LOD_COLORS[lodIndex % 4]; 
    vec3 instanceColor = idToColor(entityId ^ hash(meshIndex));
    vec3 finalColor = mix(baseLODColor, instanceColor, 0.15);

    if (int(fragCoord.x + fragCoord.y) % 8 < 4) {
        finalColor *= 0.2;
    }

    return finalColor;
}

void main() 
{ 
    uint entityId = inId.x;
    uint materialId = inId.y;
    uint meshIndex = inId.z;
    uint lodIndex = inId.w;

    MaterialBuffer globalMaterials = MaterialBuffer(pc.materialBuffer); 
    GpuMaterial mat = globalMaterials.data[materialId];

    vec2 finalUV = inUV * mat.uvScale;

    //Albedo
    vec4 color = mat.color;
    if(mat.albedoTexture != 0xFFFFFFFFu) {
        color *= sampleLoadedTexture2D(mat.albedoTexture, 0, finalUV);
    }

    //Normals
    vec3 normal = normalize(inNormal);
    if(mat.normalTexture != 0xFFFFFFFFu) {
        vec3 tangent = normalize(inTangent.xyz);
        tangent = normalize(tangent - normal * dot(normal, tangent));
        vec3 bitangent = cross(normal, tangent) * inTangent.w;
        mat3 TBN = mat3(tangent, bitangent, normal);

        vec3 normalMapSample = sampleLoadedTexture2D(mat.normalTexture, 0, finalUV).rgb;
        vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
        normal = normalize(TBN * tangentSpaceNormal);
    }

    if (color.a < 0.05) {
        discard;
    }

    outFragColor = vec4(getDebugColor(entityId, meshIndex, lodIndex, gl_FragCoord.xy), 1.0);
    outEntityId = entityId;
}