#ifndef SYN_INCLUDES_UTILS_COLOR_MATH_GLSL
#define SYN_INCLUDES_UTILS_COLOR_MATH_GLSL

const vec3 LOD_COLORS[4] = vec3[](
    vec3(0.1, 0.9, 0.2), // LOD 0 -> Green
    vec3(0.9, 0.9, 0.1), // LOD 1 -> Yellow
    vec3(0.9, 0.5, 0.1), // LOD 2 -> Orange
    vec3(0.9, 0.1, 0.1)  // LOD 3 -> Red
);

const vec2 MAT_TYPE_COLORS[4] = vec2[](
    vec2(1.0, 0.0), // 0: Opaque 1-Sided      -> Red
    vec2(0.0, 1.0), // 1: Opaque 2-Sided      -> Green
    vec2(1.0, 1.0), // 2: Transparent 1-Sided -> Yellow
    vec2(1.0, 0.4)  // 3: Transparent 2-Sided -> Orange
);

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

vec3 idToColor(uint id) {
    uint h = hash(id);
    return vec3(
        float((h >> 16) & 0xFF) / 255.0,
        float((h >> 8) & 0xFF) / 255.0,
        float(h & 0xFF) / 255.0
    );
}

vec3 getDebugColor(uint entityId, uint meshIndex, uint lodIndex, vec2 fragCoord) {
    vec3 baseLODColor = LOD_COLORS[lodIndex % 4];
    vec3 instanceColor = idToColor(entityId ^ hash(meshIndex));
    vec3 finalColor = mix(baseLODColor, instanceColor, 0.15);
    
    if (int(fragCoord.x + fragCoord.y) % 8 < 4) {
        finalColor *= 0.2;
    }
    return finalColor;
}

vec3 getMeshletLodColor(uint entityId, uint meshletIndex, uint lodIndex) {
    vec3 baseLODColor = LOD_COLORS[lodIndex % 4]; 
    
    uint combinedHash = hash(entityId ^ hash(meshletIndex));
    
    vec3 meshletColor = vec3(
        float((combinedHash >> 16) & 0xFF) / 255.0,
        float((combinedHash >> 8) & 0xFF) / 255.0,
        float(combinedHash & 0xFF) / 255.0
    );
    
    vec3 finalColor = mix(baseLODColor, meshletColor, 0.4);
    float randomBrightness = float((combinedHash >> 24) & 0xFF) / 255.0;
    float brightnessScale = 0.3 + (randomBrightness * 1.2);
    
    return finalColor * brightnessScale;
}

#endif