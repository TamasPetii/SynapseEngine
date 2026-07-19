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

#endif