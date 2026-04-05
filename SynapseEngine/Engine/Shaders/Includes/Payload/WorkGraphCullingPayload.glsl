#ifndef SYN_INCLUDES_WORK_GRAPH_CULLING_PAYLOAD_H
#define SYN_INCLUDES_WORK_GRAPH_CULLING_PAYLOAD_H

struct MeshCullingPayload {
    uint entityId; // 31. bit: parentFullyInside
    uint modelIndex;
};

#endif