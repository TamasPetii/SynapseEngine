#include "BrdfLutComputePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Render/ShaderNames.h"

namespace Syn {

    struct BrdfLutPC {
        uint32_t sampleCount;
    };

    BrdfLutComputePass::BrdfLutComputePass(uint32_t sampleCount)
        : _sampleCount(sampleCount) {}

    void BrdfLutComputePass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramSync("BrdfLutGenComp", {
            ShaderNames::BrdfLut
            });
    }

    void BrdfLutComputePass::BindDescriptors(const GpuProceduralContext& context) {
        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddStorageImage(
            0,
            context.targetImage->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void BrdfLutComputePass::PushConstants(const GpuProceduralContext& context) {
        Vk::PushConstant<BrdfLutPC> pc;
        pc->sampleCount = _sampleCount;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void BrdfLutComputePass::Dispatch(const GpuProceduralContext& context) {
        uint32_t width = context.targetImage->GetConfig().width;
        uint32_t groupCount = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCount, groupCount, 1);
    }
}