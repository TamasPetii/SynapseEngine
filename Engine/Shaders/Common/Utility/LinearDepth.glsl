float ConvertDepthToLinear(float depth, float nearPlane, float farPlane) {
    return (nearPlane * farPlane) / (farPlane - depth * (farPlane - nearPlane));
}