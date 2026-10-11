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

#ifndef SYN_INCLUDES_UTILS_CULLING_UTILS_GLSL
#define SYN_INCLUDES_UTILS_CULLING_UTILS_GLSL

#include "../Common/Material.glsl"

uint GetMaterialRenderType(Material mat) {
    bool isTransparent = IS_TRANSPARENT(mat);
    bool isAlphaTested = IS_ALPHA_TESTED(mat);
    bool isDoubleSided = IS_DOUBLE_SIDED(mat);

    if (isTransparent) {
        if (isAlphaTested) return isDoubleSided ? 7 : 6;
        else               return isDoubleSided ? 5 : 4;
    } else {
        if (isAlphaTested) return isDoubleSided ? 3 : 2;
        else               return isDoubleSided ? 1 : 0;
    }
}

uint CalculateLodFromScreenSize(float screenSizePixels) {
    if (screenSizePixels > 512.0) return 0;
    if (screenSizePixels > 256.0) return 1;
    if (screenSizePixels > 128.0) return 2;
    return 3;
}

bool IsModelSimpleEnoughForFastPath(uint meshCount, uint vertexCount) {
    const uint MAX_FAST_PATH_MESHES = 8u;
    const uint MAX_FAST_PATH_VERTICES = 4096u;
    
    return (meshCount <= MAX_FAST_PATH_MESHES && vertexCount < MAX_FAST_PATH_VERTICES);
}

void GetClusterFrustumPlanes(
    ivec2 tileId, uint tileSize, float screenWidth, float screenHeight,
    float minZ, float maxZ, mat4 projInv, out vec4 planes[6]
) {
    vec2 minUV = vec2(tileId) * float(tileSize) / vec2(screenWidth, screenHeight);
    vec2 maxUV = vec2(tileId + 1) * float(tileSize) / vec2(screenWidth, screenHeight);
    
    vec2 ndcMin = minUV * 2.0 - 1.0;
    vec2 ndcMax = maxUV * 2.0 - 1.0;

    vec4 vTL = projInv * vec4(ndcMin.x, ndcMin.y, 1.0, 1.0);
    vec4 vTR = projInv * vec4(ndcMax.x, ndcMin.y, 1.0, 1.0);
    vec4 vBL = projInv * vec4(ndcMin.x, ndcMax.y, 1.0, 1.0);
    vec4 vBR = projInv * vec4(ndcMax.x, ndcMax.y, 1.0, 1.0);
    
    vec3 pTL = vTL.xyz / vTL.w;
    vec3 pTR = vTR.xyz / vTR.w;
    vec3 pBL = vBL.xyz / vBL.w;
    vec3 pBR = vBR.xyz / vBR.w;

    planes[0] = vec4(0.0, 0.0, -1.0, minZ);
    planes[1] = vec4(normalize(cross(pTR, pBR)), 0.0);
    planes[2] = vec4(normalize(cross(pBL, pTL)), 0.0);
    planes[3] = vec4(normalize(cross(pTL, pTR)), 0.0);
    planes[4] = vec4(normalize(cross(pBR, pBL)), 0.0);
    planes[5] = vec4(0.0, 0.0, 1.0, -maxZ);
}

#endif