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

#ifndef SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL
#define SYN_INCLUDES_COMMON_FORWARD_PLUS_GLSL

#include "../Core.glsl"
#include "IndirectCommand.glsl"

struct TileData {
    float minZ;
    float maxZ;
    uint sliceCount;
    uint clusterBaseOffset;
};

struct ClusterData {
    uint tileIndex;
    uint sliceIndex; 
    float minZ;
    float maxZ;
    
    uint pointLightCount;
    uint pointLightOffset;
    uint spotLightCount;
    uint spotLightOffset;
};

layout(buffer_reference, std430) restrict buffer TileGridBuffer { 
    TileData data[]; 
};

layout(buffer_reference, std430) restrict buffer ClusterListBuffer { 
    ClusterData data[]; 
};

layout(buffer_reference, std430) restrict buffer LightIndexListBuffer { 
    uint data[]; 
};


#define GET_TILE_DATA(addr, idx)         TileGridBuffer(addr).data[idx]
#define GET_CLUSTER_DATA(addr, idx)      ClusterListBuffer(addr).data[idx]
#define GET_LIGHT_INDEX(addr, idx)       LightIndexListBuffer(addr).data[idx]
#define GET_CLUSTER_COUNT(addr)          GET_VK_DISPATCH_CMD(addr).groupCountX

#endif