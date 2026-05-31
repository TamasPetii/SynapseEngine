#ifndef SYN_INCLUDES_SHADOW_TASK_PAYLOAD_H
#define SYN_INCLUDES_SHADOW_TASK_PAYLOAD_H

struct ShadowTaskPayload {
    uint drawId;
    uint instanceId;
    uint cascadeIdx;
    uint lightIdx;
    uint meshletIndices[32];
};

#endif