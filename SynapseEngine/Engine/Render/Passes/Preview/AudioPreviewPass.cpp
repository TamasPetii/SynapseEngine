#include "AudioPreviewPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Audio/AudioManager.h"
#include "Engine/Manager/PreviewManager.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include <algorithm>

namespace Syn {

    void AudioPreviewPass::Initialize() {
    }

    void AudioPreviewPass::PrepareFrame(const RenderContext& context) {
        _dirtyAudios.clear();

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

        for (uint32_t audioId : _dirtyAudios) {
            auto audioResource = audioSnapshot[audioId].resource;
            if (!audioResource) continue;

            VkViewport viewport{};
            VkRect2D scissor{};
            pm->GetViewportAndScissor(PreviewResourceType::Audio, audioId, viewport, scissor);

            uint32_t width = scissor.extent.width;
            uint32_t height = scissor.extent.height;

            std::vector<uint16_t> pixels(width * height * 4, 0);

            auto floatToHalf = [](float v) -> uint16_t {
                return glm::detail::toFloat16(v);
                };

            uint16_t bgVal = floatToHalf(0.13f);
            uint16_t alphaVal = floatToHalf(1.0f);
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

                uint16_t rVal = floatToHalf(0.26f);
                uint16_t gVal = floatToHalf(0.59f);
                uint16_t bVal = floatToHalf(0.98f);

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

            auto stagingBuffer = pm->GetScratchStagingBuffer();
            stagingBuffer->Write(pixels.data(), pixels.size() * sizeof(uint16_t));

            Vk::BufferToImageCopyInfo copyInfo{};
            copyInfo.srcBuffer = stagingBuffer->Handle();
            copyInfo.dstImage = atlas->Handle();
            copyInfo.width = width;
            copyInfo.height = height;
            copyInfo.depth = 1;
            copyInfo.imageOffset = { scissor.offset.x, scissor.offset.y, 0 };
            copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            Vk::BufferUtils::CopyBufferToImage(context.cmd, copyInfo);

            pm->MarkCompleted(PreviewResourceType::Audio, audioId);
        }

        atlas->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            false
        );
    }
}