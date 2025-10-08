uint floatToSortableUint(float f) {
    uint u = floatBitsToUint(f);
    uint mask = uint(-int(u >> 31)) | 0x80000000u;
    return u ^ mask;
}

float sortableUintToFloat(uint u) {
    uint mask = ((u >> 31) - 1u) | 0x80000000u;
    return uintBitsToFloat(u ^ mask);
}

bool TestPlaneVsAABB(vec4 plane, vec3 aabbOrigin, vec3 aabbExtents)
{
	return (-dot(aabbExtents, abs(plane.xyz)) <= dot(plane.xyz, aabbOrigin) - plane.w);
}

bool TestPlaneVsSphere(vec4 plane, vec3 sphereOrigin, float sphereRadius)
{
	return (dot(plane.xyz, sphereOrigin) - plane.w > -sphereRadius);
}

bool TestFrustumVsAABB(CameraFrustumBuffer cameraBuffer, uint cameraIndex, vec3 aabbOrigin, vec3 aabbExtents)
{
	return 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].near, aabbOrigin, aabbExtents) && 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].right, aabbOrigin, aabbExtents) && 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].left, aabbOrigin, aabbExtents) && 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].top, aabbOrigin, aabbExtents) && 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].bottom, aabbOrigin, aabbExtents) && 
	TestPlaneVsAABB(cameraBuffer.frustums[cameraIndex].far, aabbOrigin, aabbExtents);
}

bool TestFrustumVsSphere(CameraFrustumBuffer cameraBuffer, uint cameraIndex, vec3 sphereOrigin, float sphereRadius)
{
	return
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].near, sphereOrigin, sphereRadius) &&
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].right, sphereOrigin, sphereRadius) &&
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].left, sphereOrigin, sphereRadius) &&
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].top, sphereOrigin, sphereRadius) &&
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].bottom, sphereOrigin, sphereRadius) &&
	TestPlaneVsSphere(cameraBuffer.frustums[cameraIndex].far, sphereOrigin, sphereRadius);
}

bool TestConeVsSphere(uint coneIndex, vec3 sphereOrigin, float sphereRadius)
{
	return false;
}