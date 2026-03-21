#include "DefaultGpuAnimationUploader.h"

namespace Syn
{
    AnimationUploadResult DefaultGpuAnimationUploader::Upload(const GpuBatchedAnimation& data, VkCommandBuffer cmd)
    {
        AnimationUploadResult result;

        auto getAlignedSize = [](size_t size) { return (size + 255) & ~255; };

        size_t totalStagingSize = 0;
        totalStagingSize += getAlignedSize(data.vertexSkinData.size() * sizeof(VertexSkinData));
        totalStagingSize += getAlignedSize(data.nodeTransforms.size() * sizeof(glm::mat4));
        totalStagingSize += getAlignedSize(data.frameGlobalColliders.size() * sizeof(GpuMeshCollider));
        totalStagingSize += getAlignedSize(data.frameMeshColliders.size() * sizeof(GpuMeshCollider));
        totalStagingSize += getAlignedSize(data.frameMeshletColliders.size() * sizeof(GpuMeshletCollider));

        if (totalStagingSize == 0)
            return result;

        result.stagingBuffer = Vk::BufferFactory::CreateStaging(totalStagingSize);
        size_t currentStagingOffset = 0;

        auto uploadVector = [&](const auto& vec, VkBufferUsageFlags usageFlags, std::unique_ptr<Vk::Buffer>& outBuffer)
            {
                if (vec.empty())
                    return;

                size_t byteSize = vec.size() * sizeof(vec[0]);

                outBuffer = Vk::BufferFactory::CreateGpu(byteSize, usageFlags);

                result.stagingBuffer->Write(vec.data(), byteSize, currentStagingOffset);

                Vk::BufferCopyInfo copyInfo{};
                copyInfo.srcBuffer = result.stagingBuffer->Handle();
                copyInfo.dstBuffer = outBuffer->Handle();
                copyInfo.size = byteSize;
                copyInfo.srcOffset = currentStagingOffset;
                copyInfo.dstOffset = 0;
                Vk::BufferUtils::CopyBuffer(cmd, copyInfo);

                Vk::BufferBarrierInfo barrier{};
                barrier.buffer = outBuffer->Handle();
                barrier.size = byteSize;
                barrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                barrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                barrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                barrier.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT;
                Vk::BufferUtils::InsertBarrier(cmd, barrier);

                currentStagingOffset += getAlignedSize(byteSize);
            };

        const VkBufferUsageFlags ssboFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        uploadVector(data.vertexSkinData, ssboFlags, result.hardwareBuffers.vertexSkinData);
        uploadVector(data.nodeTransforms, ssboFlags, result.hardwareBuffers.nodeTransforms);
        uploadVector(data.frameGlobalColliders, ssboFlags, result.hardwareBuffers.frameGlobalColliders);
        uploadVector(data.frameMeshColliders, ssboFlags, result.hardwareBuffers.frameMeshColliders);
        uploadVector(data.frameMeshletColliders, ssboFlags, result.hardwareBuffers.frameMeshletColliders);

        return result;
    }
}