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

#ifndef SYN_INCLUDES_WORK_GRAPH_CULLING_PAYLOAD_H
#define SYN_INCLUDES_WORK_GRAPH_CULLING_PAYLOAD_H

struct ChunkCullingPayload {
    // Bit-packing layout:
    // GEOMETRY PASS: [Bit 31: Inside Frustum] [Bits 0-30: ChunkID]
    // DIRECTIONAL LIGHT SHADOW PASS: // [Bit 31: Inside Frustum] [Bits 28-30: LightIdx] [Bits 26-27: CascadeIdx] [Bits 0-25: ChunkID]
    uint rawChunkPayload; 
};

struct MeshCullingPayload {
    // Bit-packing layout:
    // GEOMETRY PASS: [Bit 31: Inside Frustum] [Bits 0-30: EntityID]
    // DIRECTIONAL LIGHT SHADOW PASS: [Bit 31: Inside Frustum] [Bits 28-30: LightIdx] [Bits 26-27: CascadeIdx] [Bits 0-25: EntityID]
    uint entityId; 
    uint modelIndex;
};

#endif