#include "BloomPrefilterPass.h"
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

    #include "Engine/Shaders/Includes/PushConstants/BloomPrefilterPC.glsl"

    void BloomPrefilterPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("BloomPrefilter", { 
            ShaderNames::BloomPrefilter 
            });
    }

    void BloomPrefilterPass::PrepareFrame(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto main = rt->GetImage(RenderTargetNames::Main);
        auto bloom = rt->GetImage(RenderTargetNames::Bloom);

        _imageTransitions.push_back({ 
                main,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                VK_ACCESS_SHADER_READ_BIT
            });
        
        _imageTransitions.push_back({ 
            bloom,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT
            });
    }

    void BloomPrefilterPass::BindDescriptors(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto main = rt->GetImage(RenderTargetNames::Main);
        auto bloom = rt->GetImage(RenderTargetNames::Bloom);
        auto sampler = ServiceLocator::GetImageManager()->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter writer;

        writer.AddCombinedImageSampler(
            0,
            main->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        std::string mip0ViewName = 
            std::string(Vk::ImageViewNames::Default) +
            std::string(Vk::ImageViewNames::Mip) +
            "0";

        writer.AddStorageImage(
            1,
            bloom->GetView(mip0ViewName),
            VK_IMAGE_LAYOUT_GENERAL
        );

        writer.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void BloomPrefilterPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t fIdx = context.frameIndex;
        auto compManager = scene->GetComponentBufferManager();
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, fIdx);

        uint32_t width = rt->GetWidth();
        uint32_t height = rt->GetHeight();

        BloomPrefilterPC pc{};
        pc.knee = _config.knee;
        pc.threshold = _config.threshold;
        pc.texelSize = 1.0f / glm::vec2(width, height);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(BloomPrefilterPC), &pc);
    }

    void BloomPrefilterPass::Dispatch(const RenderContext& context) {
        auto rt = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t width = rt->GetWidth();
        uint32_t height = rt->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);
    }
}