#ifndef SYN_INCLUDES_UTILS_PBR_MATH_GLSL
#define SYN_INCLUDES_UTILS_PBR_MATH_GLSL

#include "../Core.glsl"

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 ShadePhysicallyBased(vec3 albedo, vec3 normal, vec3 toEye, vec3 toLight, float roughness, float metalness, vec3 lightColor, float attenuation, float strength) {
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metalness);

    vec3 N = normal;
    vec3 V = toEye;
    vec3 L = toLight;
    vec3 H = normalize(V + L);

    vec3 radiance = lightColor * attenuation * strength;   
        
    float NDF = DistributionGGX(N, H, roughness);        
    float G = GeometrySmith(N, V, L, roughness);      
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);        
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;      
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
            
    float NdotL = max(dot(N, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

#endif