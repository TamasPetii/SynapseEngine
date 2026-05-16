#ifndef SYN_INCLUDES_UTILS_ATOMIC_FLOAT_UTILS_GLSL
#define SYN_INCLUDES_UTILS_ATOMIC_FLOAT_UTILS_GLSL

uint floatToSortableUint(float f) {
    uint u = floatBitsToUint(f);
    uint mask = uint(-int(u >> 31)) | 0x80000000u;
    return u ^ mask;
}

float sortableUintToFloat(uint u) {
    uint mask = ((u >> 31) - 1u) | 0x80000000u;
    return uintBitsToFloat(u ^ mask);
}

#endif