#ifndef SYN_INCLUDES_PUSH_CONSTANTS_SELECTION_OUTLINE_PC_GLSL
#define SYN_INCLUDES_PUSH_CONSTANTS_SELECTION_OUTLINE_PC_GLSL

#include "../SharedGpuTypes.glsl"

struct SelectionOutlinePC {
    vec4 outlinePrimaryColor;
    vec4 outlineSecondaryColor;
    uint64_t frameGlobalContextBufferAddr;
    uint enableSelectedOutline;
    uint enableSelectedHierarchyOutline;
    float outlineThickness;
};

#endif