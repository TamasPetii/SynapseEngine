#ifndef SYN_INCLUDES_TASK_PAYLOAD_H
#define SYN_INCLUDES_TASK_PAYLOAD_H

struct TaskPayload {
    uint drawId;
    uint entityId;
    uint transformDenseIdx;
    uint activeCameraDenseIdx;
    uint meshletIndices[32];
};

#endif