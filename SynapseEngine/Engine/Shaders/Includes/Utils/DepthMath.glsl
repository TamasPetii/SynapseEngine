#ifndef SYN_INCLUDES_UTILS_DEPTH_MATH_GLSL
#define SYN_INCLUDES_UTILS_DEPTH_MATH_GLSL

// Converts non-linear hardware depth to linear view-space depth
float ConvertDepthToLinear(float depth, float nearPlane, float farPlane) {
    return (nearPlane * farPlane) / (farPlane - depth * (farPlane - nearPlane));
}

// Converts non-linear hardware depth to a 0..1 normalized linear depth
float ConvertDepthToLinearNormalized(float depth, float nearPlane, float farPlane) {
    float linear = ConvertDepthToLinear(depth, nearPlane, farPlane);
    return (linear - nearPlane) / (farPlane - nearPlane);
}

// Reconstructs the 3D world-space position from 2D UV coordinates and hardware depth
vec3 ReconstructWorldPosition(vec2 uv, float depth, mat4 viewProjInv) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = viewProjInv * ndc;
    return worldPos.xyz / worldPos.w;
}

#endif