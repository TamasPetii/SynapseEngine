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

#ifndef SYN_INCLUDES_UTILS_CLUSTER_MATH_GLSL
#define SYN_INCLUDES_UTILS_CLUSTER_MATH_GLSL

vec3 ScreenToView(vec4 screenCoords, mat4 projVulkanInv) {
    vec4 view = projVulkanInv * screenCoords;
    return view.xyz / view.w;
}

void GetClusterViewSpaceAABB(
    uint tileIndex, uint tileCountX, uint tileSize, float screenWidth, float screenHeight,
    float minZ, float maxZ, mat4 projVulkanInv, 
    out vec3 aabbMin, out vec3 aabbMax) 
{
    uint tileX = tileIndex % tileCountX;
    uint tileY = tileIndex / tileCountX;

    // 1. Calculate NDC coordinates [-1, 1] for the 4 corners of the Tile
    vec2 minScreen = vec2(tileX * tileSize, tileY * tileSize) / vec2(screenWidth, screenHeight);
    vec2 maxScreen = vec2((tileX + 1) * tileSize, (tileY + 1) * tileSize) / vec2(screenWidth, screenHeight);
    
    // [0, 1] to [-1, 1] mapping (No Y-flip needed if projVulkanInv already handles Vulkan's clip space)
    minScreen = minScreen * 2.0 - 1.0;
    maxScreen = maxScreen * 2.0 - 1.0;

    // 2. Unproject corners at Far Plane (NDC Z = 1.0) to get view space rays
    vec3 p0 = ScreenToView(vec4(minScreen.x, minScreen.y, 1.0, 1.0), projVulkanInv);
    vec3 p1 = ScreenToView(vec4(maxScreen.x, minScreen.y, 1.0, 1.0), projVulkanInv);
    vec3 p2 = ScreenToView(vec4(minScreen.x, maxScreen.y, 1.0, 1.0), projVulkanInv);
    vec3 p3 = ScreenToView(vec4(maxScreen.x, maxScreen.y, 1.0, 1.0), projVulkanInv);

    // 3. Scale rays linearly by the cluster's specific minZ and maxZ depths
    float minRatio = minZ / abs(p0.z);
    float maxRatio = maxZ / abs(p0.z);

    vec3 minP0 = p0 * minRatio; vec3 maxP0 = p0 * maxRatio;
    vec3 minP1 = p1 * minRatio; vec3 maxP1 = p1 * maxRatio;
    vec3 minP2 = p2 * minRatio; vec3 maxP2 = p2 * maxRatio;
    vec3 minP3 = p3 * minRatio; vec3 maxP3 = p3 * maxRatio;

    // 4. Construct the bounding box enclosing all 8 points
    aabbMin = min(min(min(minP0, minP1), min(minP2, minP3)), min(min(maxP0, maxP1), min(maxP2, maxP3)));
    aabbMax = max(max(max(minP0, minP1), max(minP2, minP3)), max(max(maxP0, maxP1), max(maxP2, maxP3)));
}

uint CalculateClusterSliceCount(float tileMinZ, float tileMaxZ, float sliceScaleFactor) {
    float range = tileMaxZ - tileMinZ;
    if (range < 0.1) return 1;

    float ratio = tileMaxZ / tileMinZ;
    uint slices = uint(log2(ratio) * sliceScaleFactor);
    
    return clamp(slices + 1, 1, 16);
}

uint GetClusterSliceIndex(float viewSpaceDepth, float tileMinZ, float tileMaxZ, uint sliceCount) {
    if (sliceCount == 0) return 0;
    if (viewSpaceDepth <= tileMinZ) return 0;
    if (viewSpaceDepth >= tileMaxZ) return sliceCount - 1;
    if (sliceCount <= 1) return 0;

    float ratio = tileMaxZ / tileMinZ;
    float logTileRatio = log2(ratio);
    float logDepthRatio = log2(viewSpaceDepth / tileMinZ);
    
    uint sliceIdx = uint((logDepthRatio / logTileRatio) * float(sliceCount));

    return clamp(sliceIdx, 0, sliceCount - 1);
}

#endif