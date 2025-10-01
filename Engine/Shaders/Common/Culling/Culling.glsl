bool IsAABBVisibleInFrustum(CameraFrustumBuffer cameraBuffer, uint cameraIndex, vec3 aabbOrigin, vec3 aabbExtents)
{	
	bool visible = true;

	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].near.xyz))   <= dot(cameraBuffer.frustums[cameraIndex].near.xyz, aabbOrigin)    - cameraBuffer.frustums[cameraIndex].near.w);
	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].right.xyz))  <= dot(cameraBuffer.frustums[cameraIndex].right.xyz, aabbOrigin)   - cameraBuffer.frustums[cameraIndex].right.w);
	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].left.xyz))   <= dot(cameraBuffer.frustums[cameraIndex].left.xyz, aabbOrigin)    - cameraBuffer.frustums[cameraIndex].left.w);
	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].top.xyz))    <= dot(cameraBuffer.frustums[cameraIndex].top.xyz, aabbOrigin)     - cameraBuffer.frustums[cameraIndex].top.w);
	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].bottom.xyz)) <= dot(cameraBuffer.frustums[cameraIndex].bottom.xyz, aabbOrigin)  - cameraBuffer.frustums[cameraIndex].bottom.w);
	visible = visible && (-dot(aabbExtents, abs(cameraBuffer.frustums[cameraIndex].far.xyz))    <= dot(cameraBuffer.frustums[cameraIndex].far.xyz, aabbOrigin)     - cameraBuffer.frustums[cameraIndex].far.w);

	return visible;
}

bool IsSphereVisibleInFrustum(CameraFrustumBuffer cameraBuffer, uint cameraIndex, vec3 origin, float radius)
{
	bool visible = true;

	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].near.xyz, origin)   - cameraBuffer.frustums[cameraIndex].near.w > -radius);
	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].right.xyz, origin)  - cameraBuffer.frustums[cameraIndex].right.w > -radius);
	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].left.xyz, origin)   - cameraBuffer.frustums[cameraIndex].left.w > -radius);
	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].top.xyz, origin)    - cameraBuffer.frustums[cameraIndex].top.w > -radius);
	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].bottom.xyz, origin) - cameraBuffer.frustums[cameraIndex].bottom.w > -radius);
	visible = visible && (dot(cameraBuffer.frustums[cameraIndex].far.xyz, origin)    - cameraBuffer.frustums[cameraIndex].far.w > -radius);

	return visible;
}