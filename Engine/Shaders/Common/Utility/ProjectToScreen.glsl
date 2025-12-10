
bool ProjectSphere(CameraBuffer cameraBuffer, uint cameraIndex, vec3 center, float radius, out vec4 projectedAABB, out float projectedLinearDepth)
{
    float znear = cameraBuffer.cameras[cameraIndex].params.x;
    float zfar = cameraBuffer.cameras[cameraIndex].params.y;
    float P00 = cameraBuffer.cameras[cameraIndex].proj[0][0];
    float P11 = cameraBuffer.cameras[cameraIndex].proj[1][1];
    vec3 centerView = (cameraBuffer.cameras[cameraIndex].view * vec4(center, 1.0)).xyz;

    if (-centerView.z < znear + radius)
        return false;

    vec3 cx = -centerView;   
    vec2 vx = vec2(sqrt(dot(cx.xz, cx.xz) - radius * radius), radius);
    vec2 minx = mat2(vx.x, vx.y, -vx.y, vx.x) * cx.xz;
    vec2 maxx = mat2(vx.x, -vx.y, vx.y, vx.x) * cx.xz;

    vec3 cy = -centerView;
    vec2 vy = vec2(sqrt(dot(cy.yz, cy.yz) - radius * radius), radius);
    vec2 miny = mat2(vy.x, vy.y, -vy.y, vy.x) * cy.yz;
    vec2 maxy = mat2(vy.x, -vy.y, vy.y, vy.x) * cy.yz;

    vec4 tempAABB = vec4(
        -(minx.x / minx.y * P00), 
        (miny.x / miny.y * P11),
        -(maxx.x / maxx.y * P00), 
        (maxy.x / maxy.y * P11)
    );

    //vec4 tempAABB = vec4(minx.x / minx.y * P00, miny.x / miny.y * P11, maxx.x / maxx.y * P00, maxy.x / maxy.y * P11);
    
    vec2 finalMin = min(tempAABB.xy, tempAABB.zw);
    vec2 finalMax = max(tempAABB.xy, tempAABB.zw);

    projectedAABB = vec4(finalMin, finalMax) * 0.5 + 0.5;

    float linearDist = -centerView.z - radius;
    projectedLinearDepth = (linearDist - znear) / (zfar - znear);

    return true;
}

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
    projectedAABB = projectedAABB * 0.5 + 0.5;
    projectedLinearDepth = (minW - znear) / (zfar - znear);

    return true;
}