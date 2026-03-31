#include "MeshCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/MeshCullingPC.glsl"

    void MeshCullingPass::Initialize() {
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("MeshCullingProgram", {
            ShaderNames::MeshCulling
            }, config);
    }

    void MeshCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) {
            _shouldDispatch = false;
            return;
        }

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        uint32_t totalModels = modelPool ? static_cast<uint32_t>(modelPool->Size()) : 0;

        if (totalModels == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;

        MeshCullingPC pc{};
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);

        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.modelCompBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);

        pc.modelAllocBufferAddr = drawData->globalModelAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.modelAddressBufferAddr = modelManager->GetModelAddressBuffer()->GetDeviceAddress();

        pc.visibleModelListAddr = compManager->GetBufferAddr(BufferNames::ModelVisibleData, fIdx);
        pc.visibleModelCountAddr = drawData->globalModelComputeCountBuffer[fIdx]->GetDeviceAddress();

        pc.meshAllocBufferAddr = drawData->globalMeshAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandBuffers = drawData->globalIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.globalInstanceBufferAddr = drawData->globalInstanceBuffers[fIdx]->GetDeviceAddress();
        
        pc.materialLookupBufferAddr = drawData->globalMaterialIndexBuffers[fIdx]->GetDeviceAddress();
        pc.materialBufferAddr = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        pc.totalModelsToTest = totalModels;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.traditionalCommandCount = drawData->activeTraditionalCount;

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(MeshCullingPC), &pc);
    }

    void MeshCullingPass::BindDescriptors(const RenderContext& context) {
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

    void MeshCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene || !scene->GetSceneDrawData()->useGpuCulling || !_shouldDispatch) return;

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        vkCmdDispatchIndirect(context.cmd, drawData->globalModelComputeCountBuffer[fIdx]->Handle(), 0);

        Vk::BufferBarrierInfo drawCmdBarrier{};
        drawCmdBarrier.buffer = drawData->globalIndirectCommandBuffers[fIdx]->Handle();
        drawCmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawCmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        drawCmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawCmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, drawCmdBarrier);

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->globalInstanceBuffers[fIdx]->Handle();
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);
    }
}