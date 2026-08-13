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

#include "AudioPreviewPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Manager/PreviewManager.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace Syn {

    void AudioPreviewPass::Initialize() {}

    void AudioPreviewPass::PrepareFrame(const RenderContext& context) {
        _dirtyAudios.clear();
        _imageTransitions.clear();

        auto pm = ServiceLocator::Get<PreviewManager>();
        _dirtyAudios = pm->GetDirtyResources(PreviewResourceType::Audio);

        if (!_dirtyAudios.empty()) {
            _imageTransitions.push_back({
                .image = pm->GetAtlasImage(),
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                .dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                .discardContent = false
                });
        }
    }

    void AudioPreviewPass::Transfer(const RenderContext& context) {
        if (_dirtyAudios.empty()) return;

        auto pm = ServiceLocator::Get<PreviewManager>();
        auto audioManager = ServiceLocator::Get<AudioManager>();
        auto atlas = pm->GetAtlasImage();
        auto audioSnapshot = audioManager->GetResourceSnapshot();

        struct TileData {
            uint32_t audioId;
            std::vector<uint16_t> pixels;
            VkRect2D scissor;
        };

        std::vector<TileData> generatedTiles;

        auto floatToHalf = [](float v) -> uint16_t {
            return glm::detail::toFloat16(v);
            };

        uint16_t bgVal = floatToHalf(0.13f);
        uint16_t alphaVal = floatToHalf(1.0f);
        uint16_t rVal = floatToHalf(0.26f);
        uint16_t gVal = floatToHalf(0.59f);
        uint16_t bVal = floatToHalf(0.98f);

        size_t totalBytesNeeded = 0;

        for (uint32_t audioId : _dirtyAudios) {
            auto audioResource = audioSnapshot[audioId].resource;
            if (!audioResource) continue;

            VkViewport viewport{};
            VkRect2D scissor{};
            pm->GetViewportAndScissor(PreviewResourceType::Audio, audioId, viewport, scissor);

            uint32_t width = scissor.extent.width;
            uint32_t height = scissor.extent.height;

            std::vector<uint16_t> pixels(width * height * 4, 0);

            for (size_t i = 0; i < pixels.size(); i += 4) {
                pixels[i + 0] = bgVal;
                pixels[i + 1] = bgVal;
                pixels[i + 2] = bgVal;
                pixels[i + 3] = alphaVal;
            }

            const auto& cpuData = audioResource->cpuData;

            if (!cpuData.waveform.empty() && width > 0) {
                float midY = height / 2.0f;
                uint32_t waveformSize = static_cast<uint32_t>(cpuData.waveform.size());

                for (uint32_t x = 0; x < width; ++x) {
                    float t = static_cast<float>(x) / static_cast<float>(width > 1 ? width - 1 : 1);
                    uint32_t waveIndex = static_cast<uint32_t>(t * (waveformSize - 1));

                    float minAmp = cpuData.waveform[waveIndex].minAmp;
                    float maxAmp = cpuData.waveform[waveIndex].maxAmp;

                    int yMin = static_cast<int>(midY - (maxAmp * midY));
                    int yMax = static_cast<int>(midY - (minAmp * midY));

                    yMin = std::max(0, std::min(static_cast<int>(height) - 1, yMin));
                    yMax = std::max(0, std::min(static_cast<int>(height) - 1, yMax));

                    if (yMin > yMax) std::swap(yMin, yMax);

                    if (yMin == yMax) {
                        yMin = std::max(0, yMin - 1);
                        yMax = std::min(static_cast<int>(height) - 1, yMax + 1);
                    }

                    for (int y = yMin; y <= yMax; ++y) {
                        int index = (y * width + x) * 4;
                        pixels[index + 0] = rVal;
                        pixels[index + 1] = gVal;
                        pixels[index + 2] = bVal;
                        pixels[index + 3] = alphaVal;
                    }
                }
            }

            totalBytesNeeded += pixels.size() * sizeof(uint16_t);
            generatedTiles.push_back({ audioId, std::move(pixels), scissor });
        }

        if (generatedTiles.empty()) return;

        std::vector<uint16_t> masterBuffer;
        masterBuffer.reserve(totalBytesNeeded / sizeof(uint16_t));

        for (const auto& tile : generatedTiles) {
            masterBuffer.insert(masterBuffer.end(), tile.pixels.begin(), tile.pixels.end());
        }

        auto stagingBuffer = Vk::BufferFactory::CreateStaging(totalBytesNeeded);
        stagingBuffer->Write(masterBuffer.data(), totalBytesNeeded);

        VkDeviceSize currentBufferOffset = 0;

        for (const auto& tile : generatedTiles) {
            Vk::BufferToImageCopyInfo copyInfo{};
            copyInfo.srcBuffer = stagingBuffer->Handle();
            copyInfo.dstImage = atlas->Handle();
            copyInfo.width = tile.scissor.extent.width;
            copyInfo.height = tile.scissor.extent.height;
            copyInfo.depth = 1;
            copyInfo.bufferOffset = currentBufferOffset;
            copyInfo.imageOffset = { tile.scissor.offset.x, tile.scissor.offset.y, 0 };
            copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            Vk::BufferUtils::CopyBufferToImage(context.cmd, copyInfo);

            currentBufferOffset += tile.pixels.size() * sizeof(uint16_t);
            pm->MarkCompleted(PreviewResourceType::Audio, tile.audioId);
        }

        pm->AddStaleBuffer(std::move(stagingBuffer));

        atlas->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            false
        );
    }
}