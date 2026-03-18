#include "MeshCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {

    struct CullingPushConstants {
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

    void MeshCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("MeshCullingProgram", {
            ShaderNames::MeshCulling
            });
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
        uint32_t fIdx = context.frameIndex;

        CullingPushConstants pc{};
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

        pc.totalModelsToTest = totalModels;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.meshletOffsetStart = SceneDrawData::MESHLET_OFFSET_START;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(CullingPushConstants), &pc);
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
        drawCmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        drawCmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
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