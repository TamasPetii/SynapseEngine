#ifndef SYN_INCLUDES_COMMON_FRAME_CONTEXT_GLSL
#define SYN_INCLUDES_COMMON_FRAME_CONTEXT_GLSL

#include "../Core.glsl"
#include "../SharedGpuTypes.glsl"

struct FrameGlobalContext {
    uint64_t globalDrawCountBufferAddr; 
    uint64_t globalInstanceIndexBufferAddr; 
    uint64_t globalIndirectCommandBufferAddr; 
    uint64_t globalIndirectCommandDescriptorBufferAddr;   
    uint64_t globalModelAllocationBufferAddr;
    uint64_t globalMeshAllocationBufferAddr; 
    
    uint64_t cameraVisibleIndexBufferAddr;
    uint64_t cameraBufferAddr;
    uint64_t cameraSparseMapBufferAddr;

    uint64_t transformBufferAddr;
    uint64_t transformSparseMapBufferAddr;
    uint64_t transformModelLinkBufferAddr;

    uint64_t staticChunkDataBufferAddr;
    uint64_t staticChunkVisibleIndexBufferAddr;
    uint64_t staticChunkCountBufferAddr;

    uint64_t modelAddressBufferAddr; 
    uint64_t modelBufferAddr;
    uint64_t modelSparseMapBufferAddr;
    uint64_t modelCountBufferAddr;
    uint64_t modelVisibleIndexBufferAddr;

    uint64_t animationAddressBufferAddr;
    uint64_t animationBufferAddr;
    uint64_t animationSparseMapBufferAddr;

    uint64_t materialLookupBufferAddr; 
    uint64_t materialBufferAddr; 

    uint64_t directionLightIndirectCommandBufferAddr;
    uint64_t directionLightVisibleIndexBufferAddr;
    uint64_t directionLightDataBufferAddr;
    uint64_t directionLightSparseMapBufferAddr;
    uint64_t directionLightShadowSparseMapBufferAddr;
    uint64_t directionLightShadowDataBufferAddr;

    uint64_t pointLightIndirectCommandBufferAddr;
    uint64_t pointLightVisibleIndexBufferAddr;
    uint64_t pointLightDataBufferAddr;
    uint64_t pointLightColliderBufferAddr;
    uint64_t pointLightSparseMapBufferAddr;
    uint64_t pointLightShadowSparseMapBufferAddr;
    uint64_t pointLightShadowDataBufferAddr; 

    uint64_t spotLightIndirectCommandBufferAddr;
    uint64_t spotLightVisibleIndexBufferAddr;
    uint64_t spotLightDataBufferAddr;
    uint64_t spotLightColliderBufferAddr;
    uint64_t spotLightSparseMapBufferAddr;
    uint64_t spotLightShadowSparseMapBufferAddr;
    uint64_t spotLightShadowDataBufferAddr;  

    uint64_t forwardPlusTileGridListBufferAddr;
    uint64_t forwardPlusClusterCountBufferAddr;
    uint64_t forwardPlusClusterListBufferAddr;
    uint64_t forwardPlusPointLightIndexListBufferAddr;
    uint64_t forwardPlusSpotLightIndexListBufferAddr;

    uint64_t wireframeMeshletInstanceIndexBufferAddr;
    uint64_t wireframeMeshAabbIndirectCommandBufferAddr;
    uint64_t wireframeMeshSphereIndirectCommandBufferAddr;

    float screenWidth;
    float screenHeight;
    float ambientStrength;
    float emissiveStrength;
    float alphaLimitDiscard;

    uint enableMeshletConeCulling;

    uint enableChunkFrustumCulling;
    uint enableModelFrustumCulling;
    uint enableMeshFrustumCulling;
    uint enableMeshletFrustumCulling;
    uint enablePointLightFrustumCulling;
    uint enableSpotLightFrustumCulling;

    uint enableChunkOcclusionCulling;
    uint enableModelOcclusionCulling;
    uint enableMeshOcclusionCulling;
    uint enableMeshletOcclusionCulling;
    uint enablePointLightOcclusionCulling;
    uint enableSpotLightOcclusionCulling;

    uint enableForwardPlusEmissiveAo;
    uint enableForwardPlusPointLights;
    uint enableForwardPlusSpotLights;
    uint enableForwardPlusDirectionalLights;

    uint globalIndirectCommandCount;
    uint globalTraditionalCommandsCount;
    uint globalMeshletCommandsCount;

    uint mainCameraEntity;
    uint activeCameraEntity;

    uint staticChunkCount;
    uint modelCount;
    uint directionLightCount;
    uint pointLightCount;
    uint spotLightCount;

    uint enableStaticBvhCulling;
    uint allTransformCount;
    uint staticTransformCount;
    uint dynamicTransformCount;
    uint streamTransformCount;
    uint nonStaticTransformCount;

    uint tileSize;
    uint tileCountX;
    uint tileCountY;
    float hizMipLevel;
    float sliceScaleFactor;
};

#ifndef __cplusplus

layout(buffer_reference, std430) readonly restrict buffer FrameContextBuffer { 
    FrameGlobalContext data; 
};

#define GET_FRAME_CONTEXT(addr) FrameContextBuffer(addr).data

#endif

#endif