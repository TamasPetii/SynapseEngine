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