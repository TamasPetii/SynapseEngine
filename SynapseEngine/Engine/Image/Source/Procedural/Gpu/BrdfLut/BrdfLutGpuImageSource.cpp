#include "BrdfLutGpuImageSource.h"
#include "BrdfLutComputePass.h"

namespace Syn
{
    BrdfLutGpuImageSource::BrdfLutGpuImageSource(uint32_t resolution, uint32_t sampleCount)
        : ProceduralImageSource("BrdfLutGpu"), _resolution(resolution), _sampleCount(sampleCount)
    {
        _pipeline = std::make_shared<GpuProceduralPipeline>();
        _pipeline->AddPass(std::make_unique<BrdfLutComputePass>(_sampleCount));
        _pipeline->InitializeAll();
    }

    std::optional<RawImage> BrdfLutGpuImageSource::Produce()
    {
        RawImage image{};
        image.width = _resolution;
        image.height = _resolution;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R32G32_SFLOAT;
        image.isCompressed = false;
        image.isGpuGenerated = true;
        image.autoCache = false;

        image.gpuGeneratorCallback = [pipeline = _pipeline](VkCommandBuffer cmd, Vk::Image& targetImage) {
            GpuProceduralContext ctx{ cmd, &targetImage };
            pipeline->Execute(ctx);
            };

        return image;
    }
}