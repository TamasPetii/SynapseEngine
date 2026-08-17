#include "IrradianceBaker.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/ShaderNames.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {
    void IrradianceBaker::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramSync("IrradianceConvolution", { 
            ShaderNames::IrradianceConvolution
            });
    }

    void IrradianceBaker::BindDescriptors(const EnvironmentBakeContext& context) {
        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            context.baseCubemap->image->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.AddStorageImage(
            1,
            context.irradianceMap->image->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void IrradianceBaker::Dispatch(const EnvironmentBakeContext& context) {
        uint32_t width = context.irradianceMap->image->GetConfig().width;
        uint32_t groupCount = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image16D);
        vkCmdDispatch(context.cmd, groupCount, groupCount, 6);
    }
}