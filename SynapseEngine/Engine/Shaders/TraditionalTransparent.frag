#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; //(EntityID, MaterialID, MeshIndex, LodIndex) 

layout(location = 0) out vec4 outAccum;
layout(location = 1) out float outReveal;

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

// McGuire WBOIT
float calculateWboitWeight(float z, float alpha) {
    return clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

vec3 simulateAmbientLight(vec3 normal, vec3 albedo, float roughness, float metallic, float ao) {
    // TODO: Később ide jön az IBL (Image-Based Lighting) Diffuse (Irradiance) 
    // és Specular (Prefiltered Env Map + BRDF LUT) szimulációja!
    
    vec3 ambient = albedo * 0.05;
    return ambient * ao;
}

vec3 simulateDirectionalLight(vec3 normal, vec3 albedo, float roughness, float metallic) {
    // TODO: dirlight
    return vec3(0.0);
}

vec3 simulatePointLights(vec3 normal, vec3 albedo, float roughness, float metallic) {
    // TODO: Forward+ point
    return vec3(0.0);
}

vec3 simulateSpotLights(vec3 normal, vec3 albedo, float roughness, float metallic) {
   // TODO: Forward+ spot
    return vec3(0.0);
}

void main() 
{ 
    uint materialId = inId.y;

    MaterialBuffer globalMaterials = MaterialBuffer(pc.materialBuffer); 
    GpuMaterial mat = globalMaterials.data[materialId];

    vec2 finalUV = inUV * mat.uvScale;

    // Albedo & Alpha
    vec4 baseColor = mat.color;
    if(mat.albedoTexture != 0xFFFFFFFFu) {
        baseColor *= sampleLoadedTexture2D(mat.albedoTexture, 0, finalUV);
    }

    // Normals & TBN
    vec3 normal = normalize(inNormal);
    vec3 finalNormal = normal;
    if(mat.normalTexture != 0xFFFFFFFFu) {
        vec3 tangent = normalize(inTangent.xyz);
        tangent = normalize(tangent - finalNormal * dot(finalNormal, tangent));
        vec3 bitangent = cross(finalNormal, tangent) * inTangent.w;
        mat3 TBN = mat3(tangent, bitangent, finalNormal);

        vec3 normalMapSample = sampleLoadedTexture2D(mat.normalTexture, 0, finalUV).rgb;
        vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
        finalNormal = normalize(TBN * tangentSpaceNormal);
    }

    // Metalness & Roughness
    float finalMetalness = mat.metalness;
    float finalRoughness = mat.roughness;
    
    if(mat.metalnessTexture != 0xFFFFFFFFu) {
        finalMetalness *= sampleLoadedTexture2D(mat.metalnessTexture, 0, finalUV).r;
    }
    if(mat.roughnessTexture != 0xFFFFFFFFu) {
        finalRoughness *= sampleLoadedTexture2D(mat.roughnessTexture, 0, finalUV).r;
    }
    if(mat.metallicRoughnessTexture != 0xFFFFFFFFu) {
        vec4 mrSample = sampleLoadedTexture2D(mat.metallicRoughnessTexture, 0, finalUV);
        finalRoughness *= mrSample.g;
        finalMetalness *= mrSample.b;
    }

    finalRoughness = clamp(finalRoughness, 0.04, 1.0);

    // Emissive
    vec3 finalEmissive = mat.emissiveColor * mat.emissiveIntensity;
    if(mat.emissiveTexture != 0xFFFFFFFFu) {
        finalEmissive *= sampleLoadedTexture2D(mat.emissiveTexture, 0, finalUV).rgb;
    }

    //Ambient Occlusion
    float finalAo = mat.aoStrength;
    if(mat.ambientOcclusionTexture != 0xFFFFFFFFu) {
        finalAo *= sampleLoadedTexture2D(mat.ambientOcclusionTexture, 0, finalUV).r;
    }

    vec3 finalColor = vec3(0.0);

    finalColor += simulateAmbientLight(normal, baseColor.rgb, finalRoughness, finalMetalness, finalAo);
    finalColor += simulateDirectionalLight(normal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += simulatePointLights(normal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += simulateSpotLights(normal, baseColor.rgb, finalRoughness, finalMetalness);
    finalColor += finalEmissive;

    vec3 premultipliedColor = finalColor * baseColor.a;
    float weight = calculateWboitWeight(gl_FragCoord.z, baseColor.a);

    outAccum = vec4(premultipliedColor, baseColor.a) * weight;
    outReveal = baseColor.a;
}