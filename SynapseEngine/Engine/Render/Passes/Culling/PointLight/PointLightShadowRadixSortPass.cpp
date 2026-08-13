#include "PointLightShadowRadixSortPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Context.h"

#include <volk.h>
#include <vk_radix_sort.h>

namespace Syn {
    PointLightShadowRadixSortPass::~PointLightShadowRadixSortPass() {
        if (_radixSorter != VK_NULL_HANDLE) {
            vrdxDestroySorter(_radixSorter);
            _radixSorter = VK_NULL_HANDLE;
        }
    }

    void PointLightShadowRadixSortPass::Initialize() {
        auto vulkanContext = ServiceLocator::Get<Vk::Context>();

        VrdxSorterCreateInfo sorterInfo = {};
        sorterInfo.physicalDevice = vulkanContext->GetPhysicalDevice()->Handle();
        sorterInfo.device = vulkanContext->GetDevice()->Handle();
        vrdxCreateSorter(&sorterInfo, &_radixSorter);
    }

    bool PointLightShadowRadixSortPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();
        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void PointLightShadowRadixSortPass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t maxSortCount = static_cast<uint32_t>(drawData->PointLightShadow.drawCallKeyBuffer.GetElementCount(fIdx));
        if (maxSortCount == 0) return;

        auto& tempBuffer = drawData->PointLightShadow.radixSortTempBuffer;

        VrdxSorterStorageRequirements reqs;
        vrdxGetSorterStorageRequirements(_radixSorter, maxSortCount, VRDX_SORT_MODE_KEY_VALUE, &reqs);
        tempBuffer.UpdateCapacity(fIdx, reqs.size);

        VkBuffer keysHandle = drawData->PointLightShadow.drawCallKeyBuffer.GetHandle(fIdx);
        VkBuffer valuesHandle = drawData->PointLightShadow.sortValuesBuffer.GetHandle(fIdx);
        VkBuffer countBuffer = drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = countBuffer;
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo keysPreBarrier{};
        keysPreBarrier.buffer = keysHandle;
        keysPreBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysPreBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysPreBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysPreBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysPreBarrier);

        Vk::BufferBarrierInfo valuesPreBarrier{};
        valuesPreBarrier.buffer = valuesHandle;
        valuesPreBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesPreBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesPreBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesPreBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesPreBarrier);

        VrdxSortInfo sortInfo{};
        sortInfo.elementCount = maxSortCount;
        sortInfo.elementCountBuffer = countBuffer;
        sortInfo.elementCountOffset = 0;
        sortInfo.keysBuffer = keysHandle;
        sortInfo.keysOffset = 0;
        sortInfo.valuesBuffer = valuesHandle;
        sortInfo.valuesOffset = 0;
        sortInfo.storageBuffer = tempBuffer.GetHandle(fIdx);
        sortInfo.storageOffset = 0;
        sortInfo.queryPool = VK_NULL_HANDLE;
        sortInfo.query = 0;

        vrdxCmdSort(context.cmd, _radixSorter, &sortInfo);

        // Synchronize sorted keys for Finalize pass
        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = keysHandle;
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);

        // Synchronize sorted values for Finalize pass
        Vk::BufferBarrierInfo valuesBarrier{};
        valuesBarrier.buffer = valuesHandle;
        valuesBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        valuesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesBarrier);
    }
}