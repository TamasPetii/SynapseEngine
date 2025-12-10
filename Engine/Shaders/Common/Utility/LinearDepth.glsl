#ifndef COMMON_UTILITY_DEPTH_GLSL
#define COMMON_UTILITY_DEPTH_GLSL

float ConvertDepthToLinear(float depth, float nearPlane, float farPlane) {
    return (nearPlane * farPlane) / (farPlane - depth * (farPlane - nearPlane));
}

float ConvertDepthToLinearNormalized(float depth, float nearPlane, float farPlane) {
    float linear = ConvertDepthToLinear(depth, nearPlane, farPlane);
    return (linear - nearPlane) / (farPlane - nearPlane);
}

#endif