#include "WorkGraphCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Context.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    bool WorkGraphCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableGpuCulling;
    }

    void WorkGraphCullingPass::Initialize() {
        auto device = ServiceLocator::GetVkContext()->GetDevice()->Handle();
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("WorkGraphCullingProgram", {
            "WorkGraphModelCulling.comp",
            "WorkGraphMeshCulling.comp"
            }, config);

        auto rootShader = shaderManager->GetShader("WorkGraphModelCulling.comp");
        auto childShader = shaderManager->GetShader("WorkGraphMeshCulling.comp");

        VkShaderModuleCreateInfo rootModInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        rootModInfo.codeSize = rootShader->GetSpirv().size() * sizeof(uint32_t);
        rootModInfo.pCode = rootShader->GetSpirv().data();
        VkShaderModule rootModule;
        SYN_VK_ASSERT_MSG(vkCreateShaderModule(device, &rootModInfo, nullptr, &rootModule), "Failed to create root shader module for Work Graph");

        VkShaderModuleCreateInfo childModInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        childModInfo.codeSize = childShader->GetSpirv().size() * sizeof(uint32_t);
        childModInfo.pCode = childShader->GetSpirv().data();
        VkShaderModule childModule;
        SYN_VK_ASSERT_MSG(vkCreateShaderModule(device, &childModInfo, nullptr, &childModule), "Failed to create child shader module for Work Graph");

        VkPipelineShaderStageNodeCreateInfoAMDX rootNodeInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX };
        rootNodeInfo.pName = "main";
        rootNodeInfo.index = 0;

        VkPipelineShaderStageCreateInfo rootStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        rootStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        rootStage.module = rootModule;
        rootStage.pName = "main";
        rootStage.pNext = &rootNodeInfo;

        VkPipelineShaderStageNodeCreateInfoAMDX childNodeInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX };
        childNodeInfo.pName = "WorkGraphMeshCullingNode";
        childNodeInfo.index = 1;

        VkPipelineShaderStageCreateInfo childStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        childStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        childStage.module = childModule;
        childStage.pName = "main";
        childStage.pNext = &childNodeInfo;

        VkPipelineShaderStageCreateInfo stages[] = { rootStage, childStage };

        VkExecutionGraphPipelineCreateInfoAMDX graphInfo{ VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX };
        graphInfo.stageCount = 2;
        graphInfo.pStages = stages;
        graphInfo.layout = _shaderProgram->GetLayout();

        SYN_VK_ASSERT_MSG(vkCreateExecutionGraphPipelinesAMDX(device, VK_NULL_HANDLE, 1, &graphInfo, nullptr, &_graphPipeline), "Failed to create Work Graph Pipeline");

        vkDestroyShaderModule(device, rootModule, nullptr);
        vkDestroyShaderModule(device, childModule, nullptr);

        VkExecutionGraphPipelineScratchSizeAMDX scratchSize{ VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX };
        vkGetExecutionGraphPipelineScratchSizeAMDX(device, _graphPipeline, &scratchSize);

        if (scratchSize.maxSize > 0) {
            _scratchBuffer = Vk::BufferFactory::CreateGpu(scratchSize.maxSize, VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX);
        }

        vkGetExecutionGraphPipelineNodeIndexAMDX(device, _graphPipeline, &rootNodeInfo, &_rootNodeIndex);
    }

    void WorkGraphCullingPass::Execute(const RenderContext& context) {
        _imageTransitions.clear();

        PrepareFrame(context);

        for (const auto& transition : _imageTransitions) {
            transition.image->TransitionLayout(
                context.cmd,
                transition.newLayout,
                transition.dstStage,
                transition.dstAccess,
                transition.discardContent
            );
        }

        if (_shaderProgram) {
            BindDescriptors(context);
            PushConstants(context);
            Dispatch(context);
        }
    }

    void WorkGraphCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        _totalModelsToTest = modelPool ? static_cast<uint32_t>(modelPool->Size()) : 0;

        if (_totalModelsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        ModelMeshCullingPC pc{};
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.modelAddressBufferAddr = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.materialBufferAddr = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        pc.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.modelCompBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);

        pc.modelAllocBufferAddr = drawData->Models.modelAllocBuffer.GetAddress(fIdx, isGpu);
        pc.meshAllocBufferAddr = drawData->Models.meshAllocBuffer.GetAddress(fIdx, isGpu);
        pc.globalIndirectCommandBuffers = drawData->Models.indirectBuffer.GetAddress(fIdx, isGpu);
        pc.globalInstanceBufferAddr = drawData->Models.instanceBuffer.GetAddress(fIdx, isGpu);
        pc.materialLookupBufferAddr = drawData->Models.materialIndexBuffer.GetAddress(fIdx, isGpu);

        pc.totalModelsToTest = _totalModelsToTest;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.traditionalCommandCount = drawData->Models.activeTraditionalCount;
        pc.enableOcclusionCulling = (scene->GetSettings()->enableGpuCulling && scene->GetSettings()->enableOcclusionCulling) ? 1 : 0;

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ModelMeshCullingPC), &pc);
    }

    void WorkGraphCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void WorkGraphCullingPass::Dispatch(const RenderContext& context) {
        if (_totalModelsToTest == 0 || !_scratchBuffer) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        vkCmdBindPipeline(context.cmd, VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX, _graphPipeline);

        vkCmdInitializeGraphScratchMemoryAMDX(
            context.cmd,
            _graphPipeline,
            _scratchBuffer->GetDeviceAddress(),
            _scratchBuffer->GetSize()
        );

        Vk::BufferBarrierInfo scratchBarrier{};
        scratchBarrier.buffer = _scratchBuffer->Handle();
        scratchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        scratchBarrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
        scratchBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        scratchBarrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, scratchBarrier);

        uint32_t groupCount = ComputeGroupSize::CalculateDispatchCount(_totalModelsToTest, ComputeGroupSize::Buffer32D);

        VkDispatchGraphInfoAMDX dispatchInfo{};
        dispatchInfo.nodeIndex = _rootNodeIndex;
        dispatchInfo.payloadCount = 1;
        dispatchInfo.payloads.hostAddress = &groupCount;
        dispatchInfo.payloadStride = sizeof(uint32_t);

        VkDispatchGraphCountInfoAMDX countInfo{};
        countInfo.count = 1;
        countInfo.infos.hostAddress = &dispatchInfo;
        countInfo.stride = sizeof(VkDispatchGraphInfoAMDX);

        vkCmdDispatchGraphAMDX(
            context.cmd,
            _scratchBuffer->GetDeviceAddress(),
            _scratchBuffer->GetSize(),
            &countInfo
        );

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->Models.instanceBuffer.GetHandle(fIdx, isGpu);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->Models.indirectBuffer.GetHandle(fIdx, isGpu);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);
    }

    WorkGraphCullingPass::~WorkGraphCullingPass() {
        if (_graphPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(ServiceLocator::GetVkContext()->GetDevice()->Handle(), _graphPipeline, nullptr);
        }
    }
}