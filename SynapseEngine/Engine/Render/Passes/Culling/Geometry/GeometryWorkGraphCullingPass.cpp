#include "GeometryWorkGraphCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Rendering/ModelComponent.h"
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
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    GeometryWorkGraphCullingPass::~GeometryWorkGraphCullingPass() {
        auto device = ServiceLocator::GetVkContext()->GetDevice()->Handle();

        if (_graphPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, _graphPipeline, nullptr);
        }

        _scratchBuffer.reset();
    }

    bool GeometryWorkGraphCullingPass::ShouldExecute(const RenderContext& context) const
    {
        //Todo
        return false;
    }

    void GeometryWorkGraphCullingPass::Initialize() {
        auto device = ServiceLocator::GetVkContext()->GetDevice()->Handle();
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("GeometryWorkGraphCullingProgram", {
                    ShaderNames::GeometryWorkGraphModelCullingComp,
                    ShaderNames::GeometryWorkGraphStaticChunkCullingComp,
                    ShaderNames::GeometryWorkGraphStaticModelCullingComp,
                    ShaderNames::GeometryWorkGraphMortonChunkCullingComp,
                    ShaderNames::GeometryWorkGraphMortonModelCullingComp,
                    ShaderNames::GeometryWorkGraphMeshCullingComp
            }, config);

        std::vector<std::string> shaderFiles = {
            "GeometryWorkGraphModelCulling.comp",
            "GeometryWorkGraphStaticChunkCulling.comp",
            "GeometryWorkGraphStaticModelCulling.comp",
            "GeometryWorkGraphMortonChunkCulling.comp",
            "GeometryWorkGraphMortonModelCulling.comp",
            "GeometryWorkGraphMeshCulling.comp"
        };

        std::vector<std::string> nodeNames = {
            "GeometryWorkGraphModelCullingNode",       // 0: Dynamic Root
            "GeometryWorkGraphStaticChunkCullingNode", // 1: Static Root
            "GeometryWorkGraphStaticModelCullingNode", // 2: Internal
            "GeometryWorkGraphMortonChunkCullingNode", // 3: Morton Root
            "GeometryWorkGraphMortonModelCullingNode", // 4: Internal
            "GeometryWorkGraphMeshCullingNode"         // 5: Internal (Leaf)
        };

        std::vector<VkShaderModule> modules(6);
        std::vector<VkPipelineShaderStageCreateInfo> stages(6);
        std::vector<VkPipelineShaderStageNodeCreateInfoAMDX> nodeInfos(6);

        for (uint32_t i = 0; i < 6; ++i) {
            auto shader = shaderManager->GetShader(shaderFiles[i]);

            VkShaderModuleCreateInfo modInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
            modInfo.codeSize = shader->GetSpirv().size() * sizeof(uint32_t);
            modInfo.pCode = shader->GetSpirv().data();

            SYN_VK_ASSERT_MSG(vkCreateShaderModule(device, &modInfo, nullptr, &modules[i]), "Failed to create shader module for Work Graph");

            nodeInfos[i] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX };
            nodeInfos[i].pName = nodeNames[i].c_str();
            nodeInfos[i].index = i;

            stages[i] = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            stages[i].stage = VK_SHADER_STAGE_COMPUTE_BIT;
            stages[i].module = modules[i];
            stages[i].pName = "main";
            stages[i].pNext = &nodeInfos[i];
        }
        
        VkExecutionGraphPipelineCreateInfoAMDX graphInfo{ VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX };
        graphInfo.stageCount = static_cast<uint32_t>(stages.size());
        graphInfo.pStages = stages.data();
        graphInfo.layout = _shaderProgram->GetLayout();

        SYN_VK_ASSERT_MSG(vkCreateExecutionGraphPipelinesAMDX(device, VK_NULL_HANDLE, 1, &graphInfo, nullptr, &_graphPipeline), "Failed to create Work Graph Pipeline");
    
        for (auto module : modules) {
            vkDestroyShaderModule(device, module, nullptr);
        }

        VkExecutionGraphPipelineScratchSizeAMDX scratchSize{ VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX };
        vkGetExecutionGraphPipelineScratchSizeAMDX(device, _graphPipeline, &scratchSize);

        if (scratchSize.maxSize > 0) {
            _scratchBuffer = Vk::BufferFactory::CreateGpu(scratchSize.maxSize, VK_BUFFER_USAGE_EXECUTION_GRAPH_SCRATCH_BIT_AMDX);
        }

        vkGetExecutionGraphPipelineNodeIndexAMDX(device, _graphPipeline, &nodeInfos[0], &_dynamicModelRootIndex);
        vkGetExecutionGraphPipelineNodeIndexAMDX(device, _graphPipeline, &nodeInfos[1], &_staticChunkRootIndex);
        vkGetExecutionGraphPipelineNodeIndexAMDX(device, _graphPipeline, &nodeInfos[3], &_mortonChunkRootIndex);
    }

    void GeometryWorkGraphCullingPass::Execute(const RenderContext& context) {
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

    void GeometryWorkGraphCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        //auto transformPool = scene->GetRegistry()->GetPool<TransformComponent>();
        //_dynamicModelCount = static_cast<uint32_t>(transformPool->GetDynamicEntities().size() + transformPool->GetStreamEntities().size());
        //_staticChunkCount = drawData->Chunks.staticChunkCount;
        //_mortonChunkCount = drawData->Chunks.mortonChunkCount;

        if (_dynamicModelCount == 0 && _staticChunkCount == 0 && _mortonChunkCount == 0) 
            return;

        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ModelMeshCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryWorkGraphCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
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

    void GeometryWorkGraphCullingPass::Dispatch(const RenderContext& context) 
    {
        if ((_dynamicModelCount == 0 && _staticChunkCount == 0 && _mortonChunkCount == 0) || !_scratchBuffer) 
            return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();
        uint32_t fIdx = context.frameIndex;

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

        uint32_t dynamicGroupCount = ComputeGroupSize::CalculateDispatchCount(_dynamicModelCount, ComputeGroupSize::Buffer32D);
        uint32_t staticChunkGroupCount = ComputeGroupSize::CalculateDispatchCount(_staticChunkCount, ComputeGroupSize::Buffer32D);
        uint32_t mortonChunkGroupCount = ComputeGroupSize::CalculateDispatchCount(_mortonChunkCount, ComputeGroupSize::Buffer32D);

        std::vector<VkDispatchGraphInfoAMDX> dispatchInfos;

        // Root 1: Dynamic Model Graph
        if (_dynamicModelCount > 0) {
            VkDispatchGraphInfoAMDX info{};
            info.nodeIndex = _dynamicModelRootIndex;
            info.payloadCount = 1;
            info.payloads.hostAddress = &dynamicGroupCount;
            info.payloadStride = sizeof(uint32_t);
            dispatchInfos.push_back(info);
        }

        // Root 2: Static Chunk Graph
        if (settings->culling.geometrySpatialAcceleration == SpatialAccelerationType::StaticBvh && _staticChunkCount > 0) {
            VkDispatchGraphInfoAMDX info{};
            info.nodeIndex = _staticChunkRootIndex;
            info.payloadCount = 1;
            info.payloads.hostAddress = &staticChunkGroupCount;
            info.payloadStride = sizeof(uint32_t);
            dispatchInfos.push_back(info);
        }

        // Root 3: Morton Chunk Graph
        if (settings->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh && _mortonChunkCount > 0) {
            VkDispatchGraphInfoAMDX info{};
            info.nodeIndex = _mortonChunkRootIndex;
            info.payloadCount = 1;
            info.payloads.hostAddress = &mortonChunkGroupCount;
            info.payloadStride = sizeof(uint32_t);
            dispatchInfos.push_back(info);
        }

        if (!dispatchInfos.empty()) {
            VkDispatchGraphCountInfoAMDX countInfo{};
            countInfo.count = static_cast<uint32_t>(dispatchInfos.size());
            countInfo.infos.hostAddress = dispatchInfos.data();
            countInfo.stride = sizeof(VkDispatchGraphInfoAMDX);

            // Start all three trees (Dynamic, Static, and Morton) simultaneously!
            vkCmdDispatchGraphAMDX(
                context.cmd,
                _scratchBuffer->GetDeviceAddress(),
                _scratchBuffer->GetSize(),
                &countInfo
            );
        }

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->Models.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->Models.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        indirectBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);
    }
}