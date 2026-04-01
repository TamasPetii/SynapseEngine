#include "BloomCompositePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/BloomCompositePC.glsl"

    bool BloomCompositePass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableBloom;
    }

    void BloomCompositePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("BloomCompositeProgram", {
            ShaderNames::BloomComposite
            });
    }

    void BloomCompositePass::PrepareFrame(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto main = rt->GetImage(RenderTargetNames::Main);
        auto bloom = rt->GetImage(RenderTargetNames::Bloom);

        _imageTransitions.push_back({
            bloom,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT
            });

        _imageTransitions.push_back({
            main,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT
            });
    }

    void BloomCompositePass::BindDescriptors(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto main = rt->GetImage(RenderTargetNames::Main);
        auto bloom = rt->GetImage(RenderTargetNames::Bloom);
        auto sampler = ServiceLocator::GetImageManager()->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter writer;

        std::string mip0ViewName = 
            std::string(Vk::ImageViewNames::Default) +
            std::string(Vk::ImageViewNames::Mip) +
            "0";

        writer.AddCombinedImageSampler(
            0,
            bloom->GetView(mip0ViewName),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        writer.AddStorageImage(
            1,
            main->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        writer.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void BloomCompositePass::PushConstants(const RenderContext& context) {
        BloomCompositePC pc{};
        pc.exposure = _config.exposure;
        pc.bloomStrength = _config.bloomStrength;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(BloomCompositePC),
            &pc
        );
    }

    void BloomCompositePass::Dispatch(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t width = rt->GetWidth();
        uint32_t height = rt->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);
    }
}