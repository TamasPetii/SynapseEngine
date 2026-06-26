#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"

namespace Syn
{
    struct SYN_API GpuAnimationBuffers
    {
        std::unique_ptr<Vk::Buffer> vertexSkinData;
        std::unique_ptr<Vk::Buffer> nodeTransforms;
        std::unique_ptr<Vk::Buffer> frameGlobalColliders;
        std::unique_ptr<Vk::Buffer> frameMeshColliders;
        std::unique_ptr<Vk::Buffer> frameMeshletColliders;
    };

    struct SYN_API GpuAnimationAddresses
    {
        uint32_t isReady;
        uint32_t padding;
        VkDeviceAddress vertexSkinData;
        VkDeviceAddress nodeTransforms;
        VkDeviceAddress frameGlobalColliders;
        VkDeviceAddress frameMeshColliders;
        VkDeviceAddress frameMeshletColliders;
        GpuAnimationDescriptor descriptor;
		GpuMeshCollider globalCollider;
    };
}