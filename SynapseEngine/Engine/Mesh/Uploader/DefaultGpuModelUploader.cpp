#include "DefaultGpuModelUploader.h"

namespace Syn
{
    ModelUploadResult DefaultGpuModelUploader::Upload(const GpuBatchedModel& data, VkCommandBuffer cmd)
    {
        ModelUploadResult result;

        auto getAlignedSize = [](size_t size) { return (size + 255) & ~255; };

        size_t totalStagingSize = 0;
        totalStagingSize += getAlignedSize(data.vertexData.vertexPositions.size() * sizeof(GpuVertexPosition));
        totalStagingSize += getAlignedSize(data.vertexData.vertexAttributes.size() * sizeof(GpuVertexAttributes));
        totalStagingSize += getAlignedSize(data.indexedData.indices.size() * sizeof(uint32_t));
        totalStagingSize += getAlignedSize(data.indexedData.meshDescriptors.size() * sizeof(GpuMeshDescriptor));
        totalStagingSize += getAlignedSize(data.indexedData.meshColliders.size() * sizeof(GpuMeshCollider));
        totalStagingSize += getAlignedSize(data.indexedData.lodDescriptors.size() * sizeof(GpuMeshLodDescriptor));
        totalStagingSize += getAlignedSize(data.meshletData.vertexIndices.size() * sizeof(uint32_t));
        totalStagingSize += getAlignedSize(data.meshletData.triangleIndices.size() * sizeof(uint32_t));
        totalStagingSize += getAlignedSize(data.meshletData.meshletDescriptors.size() * sizeof(GpuMeshletDescriptor));
        totalStagingSize += getAlignedSize(data.meshletData.drawDescriptors.size() * sizeof(GpuMeshletDrawDescriptor));
        totalStagingSize += getAlignedSize(data.meshletData.meshletColliders.size() * sizeof(GpuMeshletCollider));
        totalStagingSize += getAlignedSize(data.nodeTransforms.size() * sizeof(GpuNodeTransform));

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
        const VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | ssboFlags;
        const VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | ssboFlags;

        // Geometry
        uploadVector(data.vertexData.vertexPositions, vertexFlags, result.hardwareBuffers.vertexPositions);
        uploadVector(data.vertexData.vertexAttributes, vertexFlags, result.hardwareBuffers.vertexAttributes);
        uploadVector(data.indexedData.indices, indexFlags, result.hardwareBuffers.indices);

        // Traditional Pipeline
        uploadVector(data.indexedData.meshDescriptors, ssboFlags, result.hardwareBuffers.meshDescriptors);
        uploadVector(data.indexedData.meshColliders, ssboFlags, result.hardwareBuffers.meshColliders);
        uploadVector(data.indexedData.lodDescriptors, ssboFlags, result.hardwareBuffers.lodDescriptors);

        // Meshlet Pipeline
        uploadVector(data.meshletData.vertexIndices, ssboFlags, result.hardwareBuffers.meshletVertexIndices);
        uploadVector(data.meshletData.triangleIndices, ssboFlags, result.hardwareBuffers.meshletTriangleIndices);
        uploadVector(data.meshletData.meshletDescriptors, ssboFlags, result.hardwareBuffers.meshletDescriptors);
        uploadVector(data.meshletData.drawDescriptors, ssboFlags, result.hardwareBuffers.meshletDrawDescriptors);
        uploadVector(data.meshletData.meshletColliders, ssboFlags, result.hardwareBuffers.meshletColliders);

        // Hierarchy
        uploadVector(data.nodeTransforms, ssboFlags, result.hardwareBuffers.nodeTransforms);

        return result;
    }
}