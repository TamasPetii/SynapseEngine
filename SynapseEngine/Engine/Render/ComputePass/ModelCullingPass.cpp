#include "ModelCullingPass.h"
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

namespace Syn {

    struct CullingPushConstants {
        uint64_t animationAddressBuffer;
        uint64_t animationBufferAddr;
        uint64_t animationSparseMapBufferAddr;

        uint64_t cameraBufferAddr;            
        uint64_t cameraSparseMapBufferAddr;   
        uint64_t transformBufferAddr;         
        uint64_t transformSparseMapBufferAddr;
        uint64_t modelCompBufferAddr;         

        uint64_t meshAllocBufferAddr;         
        uint64_t modelAllocBufferAddr;        
        uint64_t modelAddressBufferAddr;      
        uint64_t visibleModelListAddr;        
        uint64_t visibleModelCountAddr;       

        uint64_t globalIndirectCommandBuffers;
        uint64_t globalInstanceBufferAddr;    
        uint32_t totalModelsToTest;           
        uint32_t activeCameraEntity;          
        uint32_t meshletOffsetStart;          
    };

    void ModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("ModelCullingProgram", {
            ShaderNames::ModelCulling
            });
    }

    void ModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        _totalModelsToTest = modelPool ? static_cast<uint32_t>(modelPool->Size()) : 0;

        if (_totalModelsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;

        CullingPushConstants pc{};
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = compManager->GetComponentBuffer(BufferNames::AnimationData, fIdx).buffer->GetDeviceAddress();
        pc.animationSparseMapBufferAddr = compManager->GetComponentBuffer(BufferNames::AnimationSparseMap, fIdx).buffer->GetDeviceAddress();

        pc.cameraBufferAddr = compManager->GetComponentBuffer(BufferNames::CameraData, fIdx).buffer->GetDeviceAddress();
        pc.cameraSparseMapBufferAddr = compManager->GetComponentBuffer(BufferNames::CameraSparseMap, fIdx).buffer->GetDeviceAddress();
        pc.transformBufferAddr = compManager->GetComponentBuffer(BufferNames::TransformData, fIdx).buffer->GetDeviceAddress();
        pc.transformSparseMapBufferAddr = compManager->GetComponentBuffer(BufferNames::TransformSparseMap, fIdx).buffer->GetDeviceAddress();
        pc.modelCompBufferAddr = compManager->GetComponentBuffer(BufferNames::ModelData, fIdx).buffer->GetDeviceAddress();

        pc.modelAllocBufferAddr = drawData->globalModelAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.modelAddressBufferAddr = modelManager->GetModelAddressBuffer()->GetDeviceAddress();

        pc.visibleModelListAddr = compManager->GetComponentBuffer(BufferNames::ModelVisibleData, fIdx).buffer->GetDeviceAddress();
        pc.visibleModelCountAddr = drawData->globalModelComputeCountBuffer[fIdx]->GetDeviceAddress();

        pc.meshAllocBufferAddr = drawData->globalMeshAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandBuffers = drawData->globalIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.globalInstanceBufferAddr = drawData->globalInstanceBuffers[fIdx]->GetDeviceAddress();

        pc.totalModelsToTest = _totalModelsToTest;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.meshletOffsetStart = SceneDrawData::MESHLET_OFFSET_START;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(CullingPushConstants), &pc);
    }

    void ModelCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene || !scene->GetSceneDrawData()->useGpuCulling || _totalModelsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalModelsToTest, ComputeGroupSize::Buffer32D);

        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->globalModelComputeCountBuffer[fIdx]->Handle();
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = compManager->GetComponentBuffer(BufferNames::ModelVisibleData, fIdx).buffer->Handle();
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
    }
}