#include "MortonRadixSortPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/BufferNames.h"

#include <volk.h>
#define VRDX_IMPLEMENTATION
#include <vk_radix_sort.h>

namespace Syn {
    MortonRadixSortPass::~MortonRadixSortPass()
    {
        if (_radixSorter != VK_NULL_HANDLE) {
            vrdxDestroySorter(_radixSorter);
            _radixSorter = VK_NULL_HANDLE;
        }
    }

    void MortonRadixSortPass::Initialize() {
        auto vulkanContext = ServiceLocator::GetVkContext();
        VkPhysicalDevice physicalDevice = vulkanContext->GetPhysicalDevice()->Handle();
        VkDevice device = vulkanContext->GetDevice()->Handle();

        VrdxSorterCreateInfo sorterInfo = {};
        sorterInfo.physicalDevice = physicalDevice;
        sorterInfo.device = device;
        vrdxCreateSorter(&sorterInfo, &_radixSorter);
    }

    bool MortonRadixSortPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        return context.scene->GetSettings()->enableMortonBvhCulling && pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void MortonRadixSortPass::Transfer(const RenderContext& context) {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        _staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());

        auto drawData = context.scene->GetSceneDrawData();
        auto compManager = context.scene->GetComponentBufferManager();
        auto& tempBuffer = drawData->Chunks.mortonRadixSortTempBuffer;
        uint32_t fIdx = context.frameIndex;

        VrdxSorterStorageRequirements reqs;
        vrdxGetSorterKeyValueStorageRequirements(_radixSorter, _staticCount, &reqs);
        tempBuffer.UpdateCapacity(context.frameIndex, static_cast<uint32_t>(reqs.size));

        VkBuffer keysHandle = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        VkBuffer valuesHandle = compManager->GetComponentBuffer(BufferNames::MortonValuesData, fIdx).buffer->Handle();
        VkBuffer tempHandle = tempBuffer.GetHandle(context.frameIndex, true);

        vrdxCmdSortKeyValue(
            context.cmd,
            _radixSorter,
            _staticCount,
            keysHandle,
            0,
            valuesHandle,
            0,
            tempHandle,
            0,
            VK_NULL_HANDLE,
            0
        );

        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = keysHandle;
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);

        Vk::BufferBarrierInfo valuesBarrier{};
        valuesBarrier.buffer = valuesHandle;
        valuesBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesBarrier);
    }
}