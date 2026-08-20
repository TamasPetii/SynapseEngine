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

#include "DefaultGpuAnimationUploader.h"

namespace Syn
{
    AnimationUploadResult DefaultGpuAnimationUploader::Upload(const GpuBatchedAnimation& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader)
    {
        AnimationUploadResult result;

        auto getAlignedSize = [](size_t size) { return (size + 255) & ~255; };

        size_t totalStagingSize = 0;
        totalStagingSize += getAlignedSize(data.vertexSkinData.size() * sizeof(VertexSkinData));
        totalStagingSize += getAlignedSize(data.nodeTransforms.size() * sizeof(GpuNodeTransform));
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

                uploader->RegisterBufferTransfer({
                    .buffer = outBuffer->Handle(),
                    .size = byteSize
                });

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