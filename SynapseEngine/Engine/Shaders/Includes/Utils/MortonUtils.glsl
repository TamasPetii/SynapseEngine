#ifndef SYN_INCLUDES_UTILS_MORTON_UTILS_GLSL
#define SYN_INCLUDES_UTILS_MORTON_UTILS_GLSL

uint expandBits9(uint v) {
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

uint CalculateMortonCode27(vec3 normalizedPos) {
    uint x = min(max(uint(normalizedPos.x * 512.0), 0u), 511u);
    uint y = min(max(uint(normalizedPos.y * 512.0), 0u), 511u);
    uint z = min(max(uint(normalizedPos.z * 512.0), 0u), 511u);
    return (expandBits9(x) | (expandBits9(y) << 1) | (expandBits9(z) << 2));
}

uint CalculateSizeBin(float radius) {
    float bin = log2(radius) + 5.0; 
    return clamp(uint(max(bin, 0.0)), 0u, 15u);
}

uint PackMorton32(uint sizeBin, uint morton27) {
    return (sizeBin << 27) | (morton27 & 0x07FFFFFFu);
}

uint64_t PackMorton64(uint sizeBin, uint morton27, uint denseIndex) {
    return (uint64_t(sizeBin) << 60) | (uint64_t(morton27) << 32) | uint64_t(denseIndex);
}

#endif