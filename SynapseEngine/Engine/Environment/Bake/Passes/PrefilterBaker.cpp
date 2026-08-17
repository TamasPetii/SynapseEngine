#include "PrefilterBaker.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Render/ShaderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include <algorithm>
#include <cmath>

namespace Syn {
    struct PrefilterPC {
        float roughness;
    };

    void PrefilterBaker::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramSync("PrefilterConvolution", { 
            ShaderNames::PrefilterConvolution
        });
    }

    void PrefilterBaker::Bake(const EnvironmentBakeContext& context) 
    {
        if (!_shaderProgram && _shaderProgramId != UINT32_MAX) {
            _shaderProgram = ServiceLocator::Get<ShaderManager>()->GetResource(_shaderProgramId).get();
        }

        if (!_shaderProgram) return;

        _shaderProgram->Bind(context.cmd);

        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
        uint32_t mipLevels = context.prefilteredMap->image->GetConfig().mipLevels;

        for (uint32_t mip = 0; mip < mipLevels; ++mip) {
            Vk::PushDescriptorWriter pushWriter;

            pushWriter.AddCombinedImageSampler(
                0,
                context.baseCubemap->image->GetView(Vk::ImageViewNames::Default),
                sampler->Handle(),
                VK_IMAGE_LAYOUT_GENERAL
            );

            std::string mipViewName = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(mip);

            pushWriter.AddStorageImage(
                1,
                context.prefilteredMap->image->GetView(mipViewName),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            Vk::PushConstant<PrefilterPC> pc;
            pc->roughness = static_cast<float>(mip) / static_cast<float>(mipLevels - 1);
            pc.Push(context.cmd, _shaderProgram->GetLayout());

            uint32_t mipWidth = std::max(1u, context.prefilteredMap->image->GetConfig().width >> mip);
            uint32_t groupCount = ComputeGroupSize::CalculateDispatchCount(mipWidth, ComputeGroupSize::Image16D);

            vkCmdDispatch(context.cmd, groupCount, groupCount, 6);
        }
    }
}