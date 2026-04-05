#ifndef SYN_INCLUDES_TASK_PAYLOAD_H
#define SYN_INCLUDES_TASK_PAYLOAD_H

struct TaskPayload {
    uint drawId;
    uint instanceId;
    uint meshletIndices[32];
};

#endif