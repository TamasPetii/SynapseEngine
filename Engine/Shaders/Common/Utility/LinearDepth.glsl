float ConvertDepthToLinear(float depth, float nearPlane, float farPlane) {
    return (nearPlane * farPlane) / (farPlane - depth * (farPlane - nearPlane));
}

float ConvertDepthToLinearNormalized(float depth, float nearPlane, float farPlane) {
    float linear = ConvertDepthToLinear(depth, nearPlane, farPlane);
    return (linear - nearPlane) / (farPlane - nearPlane);
}