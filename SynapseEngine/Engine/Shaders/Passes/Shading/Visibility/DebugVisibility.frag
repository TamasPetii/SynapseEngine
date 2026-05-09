#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Utils/ColorMath.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl" 
#include "../../../Includes/Common/Model.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Transform.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform usampler2D visibilityTexture;

#include "../../../Includes/PushConstants/DebugVisibilityPC.glsl"

layout(push_constant) uniform PushConstants { 
    DebugVisibilityPC pc;
};

void main() {
    ivec2 texCoords = ivec2(gl_FragCoord.xy);
    uvec2 visData = texelFetch(visibilityTexture, texCoords, 0).xy;
    
    uint word0 = visData.x;
    uint payload = visData.y;
    uint entityId = UNPACK_VISIBILITY_ENTITY(word0);

    if (entityId == INVALID_INDEX) {
        outColor = vec4(0.05, 0.05, 0.05, 1.0);
        return;
    }

    uint pipeFlag  = UNPACK_VISIBILITY_PIPELINE(word0);
    uint lodIdx    = UNPACK_VISIBILITY_LOD(payload);
    uint meshIdx   = UNPACK_VISIBILITY_MESH(payload);

    switch (pc.debugMode) {
        case DEBUG_MODE_ENTITY_ID:
            outColor = vec4(idToColor(entityId), 1.0);
            break;

        case DEBUG_MODE_PIPELINE_TYPE:
            outColor = (pipeFlag == VIS_PIPELINE_MESH_SHADER) ? vec4(0.1, 0.8, 0.2, 1.0) : vec4(0.8, 0.1, 0.1, 1.0);
            break;

        case DEBUG_MODE_LOD_LEVEL:
            float lodG = 1.0 - (float(lodIdx) * 0.25);
            outColor = vec4(vec3(lodG), 1.0);
            break;

        case DEBUG_MODE_MESH_INDEX:
            outColor = vec4(idToColor(entityId ^ hash(meshIdx)), 1.0);
            break;

        case DEBUG_MODE_MESHLET_INDEX:
            if (pipeFlag == VIS_PIPELINE_MESH_SHADER) {
                uint msIdx = UNPACK_VISIBILITY_MS_MESHLET(payload);
                outColor = vec4(getMeshletLodColor(entityId, msIdx, lodIdx), 1.0);
            } else {
                outColor = vec4(0.1, 0.1, 0.1, 1.0);
            }
            break;

        case DEBUG_MODE_TRIANGLE_INDEX: {
            uint triIdx = (pipeFlag == VIS_PIPELINE_MESH_SHADER) ? UNPACK_VISIBILITY_MS_TRIANGLE(payload) : UNPACK_VISIBILITY_PRIMITIVE_ID(payload);
            outColor = vec4(idToColor(entityId ^ hash(triIdx)), 1.0);
            break;
        }
    }
}