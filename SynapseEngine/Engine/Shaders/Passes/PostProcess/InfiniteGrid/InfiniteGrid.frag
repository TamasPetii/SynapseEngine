// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/PushConstants/InfiniteGridPC.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
   InfiniteGridPC pc;
};

struct IntersectionData {
    float t;
    vec3 worldPos;
    vec4 color;
    bool valid;
};

float ComputeGrid(vec2 coord, float scale, float thickness) {
    vec2 scaledCoord = coord / scale;
    vec2 derivative = fwidth(scaledCoord);
    
    vec2 grid = abs(fract(scaledCoord - 0.5) - 0.5) / (derivative * thickness);
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    vec3 ndcPos = vec3(inUV * 2.0 - 1.0, 1.0); 
    vec3 ndcNear = vec3(ndcPos.xy, 0.000001);

    vec4 worldFar4 = camera.viewProjVulkanInv * vec4(ndcPos, 1.0);
    vec4 worldNear4 = camera.viewProjVulkanInv * vec4(ndcNear, 1.0);
    vec3 worldFar = worldFar4.xyz / worldFar4.w;
    vec3 worldNear = worldNear4.xyz / worldNear4.w;

    vec3 rayDir = normalize(worldFar - worldNear);
    vec3 rayOrigin = worldNear;

    IntersectionData hits[3];
    for(int i = 0; i < 3; ++i) hits[i].valid = false;

    if (rayDir.y != 0.0) {
        float t = -rayOrigin.y / rayDir.y;
        if (t > 0.0) {
            hits[0].t = t; hits[0].worldPos = rayOrigin + rayDir * t; hits[0].valid = true;
            vec2 coord = hits[0].worldPos.xz; vec4 c = vec4(0.0);

            if ((pc.planeFlags & 1) != 0) {
                float grid1m = ComputeGrid(coord, pc.gridScale, pc.gridThickness);
                float grid10m = ComputeGrid(coord, pc.gridScale * 10.0, pc.gridThickness * 1.5);
                float lodFade = 1.0 - smoothstep(10.0, 50.0, distance(camera.eye.xyz, hits[0].worldPos));
                c = mix(pc.gridColor * vec4(1.0, 1.0, 1.0, 0.5) * grid1m * lodFade, pc.gridColor * grid10m, grid10m);
            }

            vec2 derivative = fwidth(coord);
            vec2 axisLines = abs(coord) / (derivative * pc.axisThickness);
            if ((pc.axisFlags & 1) != 0) c = mix(c, pc.axisXColor, 1.0 - min(axisLines.y, 1.0));
            if ((pc.axisFlags & 4) != 0) c = mix(c, pc.axisZColor, 1.0 - min(axisLines.x, 1.0));
            
            c.a *= (1.0 - smoothstep(pc.fadeDistance * 0.1, pc.fadeDistance, t));
            hits[0].color = c;
        }
    }

    if (rayDir.z != 0.0) {
        float t = -rayOrigin.z / rayDir.z;
        if (t > 0.0) {
            hits[1].t = t; hits[1].worldPos = rayOrigin + rayDir * t; hits[1].valid = true;
            vec2 coord = hits[1].worldPos.xy; vec4 c = vec4(0.0);

            if ((pc.planeFlags & 2) != 0) {
                float grid1m = ComputeGrid(coord, pc.gridScale, pc.gridThickness);
                float grid10m = ComputeGrid(coord, pc.gridScale * 10.0, pc.gridThickness * 1.5);
                float lodFade = 1.0 - smoothstep(10.0, 50.0, distance(camera.eye.xyz, hits[1].worldPos));
                c = mix(pc.gridColor * vec4(1.0, 1.0, 1.0, 0.5) * grid1m * lodFade, pc.gridColor * grid10m, grid10m);
            }

            vec2 derivative = fwidth(coord);
            vec2 axisLines = abs(coord) / (derivative * pc.axisThickness);
            if ((pc.axisFlags & 1) != 0) c = mix(c, pc.axisXColor, 1.0 - min(axisLines.y, 1.0));
            if ((pc.axisFlags & 2) != 0) c = mix(c, pc.axisYColor, 1.0 - min(axisLines.x, 1.0));

            c.a *= (1.0 - smoothstep(pc.fadeDistance * 0.1, pc.fadeDistance, t));
            hits[1].color = c;
        }
    }

    if (rayDir.x != 0.0) {
        float t = -rayOrigin.x / rayDir.x;
        if (t > 0.0) {
            hits[2].t = t; hits[2].worldPos = rayOrigin + rayDir * t; hits[2].valid = true;
            vec2 coord = hits[2].worldPos.yz; vec4 c = vec4(0.0);

            if ((pc.planeFlags & 4) != 0) {
                float grid1m = ComputeGrid(coord, pc.gridScale, pc.gridThickness);
                float grid10m = ComputeGrid(coord, pc.gridScale * 10.0, pc.gridThickness * 1.5);
                float lodFade = 1.0 - smoothstep(10.0, 50.0, distance(camera.eye.xyz, hits[2].worldPos));
                c = mix(pc.gridColor * vec4(1.0, 1.0, 1.0, 0.5) * grid1m * lodFade, pc.gridColor * grid10m, grid10m);
            }

            vec2 derivative = fwidth(coord);
            vec2 axisLines = abs(coord) / (derivative * pc.axisThickness);
            if ((pc.axisFlags & 2) != 0) c = mix(c, pc.axisYColor, 1.0 - min(axisLines.y, 1.0));
            if ((pc.axisFlags & 4) != 0) c = mix(c, pc.axisZColor, 1.0 - min(axisLines.x, 1.0));

            c.a *= (1.0 - smoothstep(pc.fadeDistance * 0.1, pc.fadeDistance, t));
            hits[2].color = c;
        }
    }

    int sortIdx[3] = {0, 1, 2};
    if ((hits[sortIdx[0]].valid ? hits[sortIdx[0]].t : -1.0) < (hits[sortIdx[1]].valid ? hits[sortIdx[1]].t : -1.0)) { int tmp = sortIdx[0]; sortIdx[0] = sortIdx[1]; sortIdx[1] = tmp; }
    if ((hits[sortIdx[1]].valid ? hits[sortIdx[1]].t : -1.0) < (hits[sortIdx[2]].valid ? hits[sortIdx[2]].t : -1.0)) { int tmp = sortIdx[1]; sortIdx[1] = sortIdx[2]; sortIdx[2] = tmp; }
    if ((hits[sortIdx[0]].valid ? hits[sortIdx[0]].t : -1.0) < (hits[sortIdx[1]].valid ? hits[sortIdx[1]].t : -1.0)) { int tmp = sortIdx[0]; sortIdx[0] = sortIdx[1]; sortIdx[1] = tmp; }

    vec4 accumulatedColor = vec4(0.0);
    float closestDepth = 1.0;
    bool hashit = false;

    for(int i = 0; i < 3; ++i) {
        int idx = sortIdx[i];
        if (hits[idx].valid && hits[idx].color.a > 0.001) {
            accumulatedColor.rgb = mix(accumulatedColor.rgb, hits[idx].color.rgb, hits[idx].color.a);
            accumulatedColor.a = mix(accumulatedColor.a, 1.0, hits[idx].color.a);

            vec4 clipPos = camera.viewProjVulkan * vec4(hits[idx].worldPos, 1.0);
            closestDepth = clipPos.z / clipPos.w;
            hashit = true;
        }
    }

    if (!hashit || accumulatedColor.a < 0.01) discard;

    gl_FragDepth = closestDepth;
    outColor = accumulatedColor;
}