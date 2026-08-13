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
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Visibility.glsl"
#include "../../../Includes/Utils/ColorMath.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl" 
#include "../../../Includes/Common/Model.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/Transform.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/Cluster.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform usampler2D visibilityTexture;

#include "../../../Includes/PushConstants/DebugVisibilityPC.glsl"

layout(push_constant) uniform PushConstants { 
    DebugVisibilityPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    ivec2 texCoords = ivec2(gl_FragCoord.xy);
    uvec2 visData = texelFetch(visibilityTexture, texCoords, 0).xy;
    
    uint word0 = visData.x;
    uint payload = visData.y;
    uint entityId = UNPACK_VISIBILITY_ENTITY(word0);

    if (word0 == INVALID_INDEX) {
        outColor = vec4(0.05, 0.05, 0.05, 1.0);
        return;
    }

    uint pipeFlag  = UNPACK_VISIBILITY_PIPELINE(word0);
    uint lodIdx    = UNPACK_VISIBILITY_LOD(payload);
    uint meshIdx   = UNPACK_VISIBILITY_MESH(payload);

    switch (pc.debugMode) {
        case DEBUG_MODE_ENTITY_ID: {
            outColor = vec4(idToColor(entityId), 1.0);
            break;
        }

        case DEBUG_MODE_PIPELINE_TYPE: {
            outColor = (pipeFlag == VIS_PIPELINE_MESH_SHADER) ? vec4(0.1, 0.8, 0.2, 1.0) : vec4(0.8, 0.1, 0.1, 1.0);
            break;
        }

        case DEBUG_MODE_LOD_LEVEL: {
            float lodG = 1.0 - (float(lodIdx) * 0.25);
            outColor = vec4(vec3(lodG), 1.0);
            break;
        }

        case DEBUG_MODE_MESH_INDEX: {
            outColor = vec4(idToColor(entityId ^ hash(meshIdx)), 1.0);
            break;
        }

        case DEBUG_MODE_MESHLET_INDEX: {
            if (pipeFlag == VIS_PIPELINE_MESH_SHADER) {
                uint msIdx = UNPACK_VISIBILITY_MS_MESHLET(payload);
                uint combinedHash = entityId ^ hash(meshIdx ^ hash(msIdx));
                outColor = vec4(idToColor(combinedHash), 1.0);
            } else {
                outColor = vec4(0.1, 0.1, 0.1, 1.0);
            }
            break;
        }

        case DEBUG_MODE_TRIANGLE_INDEX: {
            uint triIdx = (pipeFlag == VIS_PIPELINE_MESH_SHADER) ? UNPACK_VISIBILITY_MS_TRIANGLE(payload) : UNPACK_VISIBILITY_PRIMITIVE_ID(payload);
            outColor = vec4(idToColor(entityId ^ hash(triIdx)), 1.0);
            break;
        }

        case DEBUG_MODE_ALL_COMBINED: {
            if (pipeFlag == VIS_PIPELINE_TRADITIONAL) {
                outColor = vec4(getDebugColor(entityId, meshIdx, lodIdx, gl_FragCoord.xy), 1.0);
            } else {
                uint msIdx = UNPACK_VISIBILITY_MS_MESHLET(payload);
                outColor = vec4(getMeshletLodColor(entityId, msIdx, lodIdx), 1.0);
            }
            break;
        }
        
        case DEBUG_MODE_MATERIAL_TYPE: {
            uint modelDenseIndex = GET_SPARSE_INDEX(ctx.modelSparseMapBufferAddr, entityId);
            
            if (modelDenseIndex == INVALID_INDEX) {
                outColor = vec4(1.0, 0.0, 1.0, 1.0);
                break;
            }

            ModelComponent comp = GET_MODEL_COMP(ctx.modelBufferAddr, modelDenseIndex);

            uint materialId = GET_MATERIAL_INDEX(ctx.materialLookupBufferAddr, comp.materialOffset + meshIdx);
            Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
            
            // Determine Material Render Type (Opaque/Transparent + 1/2 Sided)
            uint matType = 0; 
            if (IS_TRANSPARENT(mat) && IS_DOUBLE_SIDED(mat))       matType = 3;
            else if (IS_TRANSPARENT(mat) && !IS_DOUBLE_SIDED(mat)) matType = 2;
            else if (!IS_TRANSPARENT(mat) && IS_DOUBLE_SIDED(mat)) matType = 1;

            vec2 baseColor = MAT_TYPE_COLORS[matType % 4];
            
            float hashMod = (float(hash(materialId) & 0xFF) / 255.0) * 0.4 - 0.2;
            outColor = vec4(clamp(baseColor + hashMod, 0.0, 1.0), 0.2, 1.0);

            break;
        }

        case DEBUG_MODE_CLUSTER_SLICES: {
            uint tileX = uint(gl_FragCoord.x) / ctx.tileSize;
            uint tileY = uint(gl_FragCoord.y) / ctx.tileSize;
            uint tileIndex = tileY * ctx.tileCountX + tileX;
            TileData tile = GET_TILE_DATA(ctx.forwardPlusTileGridListBufferAddr, tileIndex);
            
            float heat = float(tile.sliceCount) / 16.0; 
            vec3 heatColor = GetHeatmapColor(heat);
            
            if (int(gl_FragCoord.x) % int(ctx.tileSize) == 0 || int(gl_FragCoord.y) % int(ctx.tileSize) == 0) {
                heatColor = vec3(1.0); 
            }
            
            outColor = vec4(heatColor, 1.0);
            break;
        }
        
        case DEBUG_MODE_CLUSTER_LIGHTS: {
            uint tileX = uint(gl_FragCoord.x) / ctx.tileSize;
            uint tileY = uint(gl_FragCoord.y) / ctx.tileSize;
            uint tileIndex = tileY * ctx.tileCountX + tileX;
            TileData tile = GET_TILE_DATA(ctx.forwardPlusTileGridListBufferAddr, tileIndex);
            
            uint totalLights = 0;
            for(uint s = 0; s < tile.sliceCount; ++s) {
                uint clusterIndex = tile.clusterBaseOffset + s;
                ClusterData cluster = GET_CLUSTER_DATA(ctx.forwardPlusClusterListBufferAddr, clusterIndex);
                totalLights += (cluster.pointLightCount + cluster.spotLightCount);
            }
            
            float heat = float(totalLights) / 64.0; 
            vec3 heatColor = GetHeatmapColor(heat);
            
            if (int(gl_FragCoord.x) % int(ctx.tileSize) == 0 || int(gl_FragCoord.y) % int(ctx.tileSize) == 0) {
                heatColor = vec3(1.0); 
            }
            
            outColor = vec4(heatColor, 1.0);
            break;   
        }
    }
}