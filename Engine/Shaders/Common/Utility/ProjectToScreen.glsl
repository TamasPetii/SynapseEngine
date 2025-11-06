/*
// 2D Polyhedral Bounds of a Clipped, Perspective-Projected 3D Sphere. Michael Mara, Morgan McGuire. 2013
bool projectSphere(vec3 C, float r, float znear, float P00, float P11, out vec4 aabb)
{
	if (C.z < r + znear)
		return false;

	vec2 cx = -C.xz;
	vec2 vx = vec2(sqrt(dot(cx, cx) - r * r), r);
	vec2 minx = mat2(vx.x, vx.y, -vx.y, vx.x) * cx;
	vec2 maxx = mat2(vx.x, -vx.y, vx.y, vx.x) * cx;

	vec2 cy = -C.yz;
	vec2 vy = vec2(sqrt(dot(cy, cy) - r * r), r);
	vec2 miny = mat2(vy.x, vy.y, -vy.y, vy.x) * cy;
	vec2 maxy = mat2(vy.x, -vy.y, vy.y, vy.x) * cy;

	aabb = vec4(minx.x / minx.y * P00, miny.x / miny.y * P11, maxx.x / maxx.y * P00, maxy.x / maxy.y * P11);
	aabb = aabb.xwzy * vec4(0.5f, -0.5f, 0.5f, -0.5f) + vec4(0.5f); // clip space -> uv space

	return true;
}
*/

bool ProjectBox(CameraBuffer cameraBuffer, uint cameraIndex, vec3 bmin, vec3 bmax, out vec4 projectedAABB, out float projectedLinearDepth)
{
    float znear = cameraBuffer.cameras[cameraIndex].params.x;
    float zfar = cameraBuffer.cameras[cameraIndex].params.y;

    // Box méretek view-projection térben
    vec4 SX = cameraBuffer.cameras[cameraIndex].viewProj * vec4(bmax.x - bmin.x, 0.0, 0.0, 0.0);
    vec4 SY = cameraBuffer.cameras[cameraIndex].viewProj * vec4(0.0, bmax.y - bmin.y, 0.0, 0.0);
    vec4 SZ = cameraBuffer.cameras[cameraIndex].viewProj * vec4(0.0, 0.0, bmax.z - bmin.z, 0.0);

    // Alap pont és 7 sarok kiszámítása
    vec4 P0 = cameraBuffer.cameras[cameraIndex].viewProj * vec4(bmin, 1.0);
    vec4 P1 = P0 + SZ;
    vec4 P2 = P0 + SY;
    vec4 P3 = P2 + SZ;
    vec4 P4 = P0 + SX;
    vec4 P5 = P4 + SZ;
    vec4 P6 = P4 + SY;
    vec4 P7 = P6 + SZ;

    // Minimum w komponens (hogy ne legyen a near plane mögött)
    float minW = min(min(min(P0.w, P1.w), min(P2.w, P3.w)),
                     min(min(P4.w, P5.w), min(P6.w, P7.w)));

    if (minW < znear)
        return false;

    // Képernyõ koordináták kiszámítása (perspective divide)
    vec2 p0 = P0.xy / P0.w;
    vec2 p1 = P1.xy / P1.w;
    vec2 p2 = P2.xy / P2.w;
    vec2 p3 = P3.xy / P3.w;
    vec2 p4 = P4.xy / P4.w;
    vec2 p5 = P5.xy / P5.w;
    vec2 p6 = P6.xy / P6.w;
    vec2 p7 = P7.xy / P7.w;

    // AABB számítás (clip space-ben)
    projectedAABB.xy = min(min(min(p0, p1), min(p2, p3)), min(min(p4, p5), min(p6, p7)));
    projectedAABB.zw = max(max(max(p0, p1), max(p2, p3)), max(max(p4, p5), max(p6, p7)));

    // Clip space -> UV space [0..1]
    projectedAABB = projectedAABB.xwzy * vec4(0.5, -0.5, 0.5, -0.5) + vec4(0.5);
    projectedLinearDepth = (minW - znear) / (zfar - znear);

    return true;
}