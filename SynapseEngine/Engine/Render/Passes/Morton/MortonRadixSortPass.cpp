#include "MortonRadixSortPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/BufferNames.h"

#define VRDX_IMPLEMENTATION
#include <vk_radix_sort.h>
#include <volk.h>

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
        return pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void MortonRadixSortPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto drawData = context.scene->GetSceneDrawData();
        auto compManager = context.scene->GetComponentBufferManager();
        auto& tempBuffer = drawData->Chunks.mortonRadixSortTempBuffer;
        uint32_t fIdx = context.frameIndex;

        VrdxSorterStorageRequirements reqs;
        vrdxGetSorterStorageRequirements(_radixSorter, _staticCount, &reqs);

        uint32_t requiredElements = static_cast<uint32_t>((reqs.size + sizeof(uint64_t) - 1) / sizeof(uint64_t));
        tempBuffer.UpdateCapacity(context.frameIndex, requiredElements);

        VkBuffer keysHandle = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        VkBuffer tempHandle = tempBuffer.GetHandle(context.frameIndex, true);

        vrdxCmdSort(
            context.cmd,
            _radixSorter,
            _staticCount,
            keysHandle,
            0,
            tempHandle,
            0,
            VK_NULL_HANDLE,
            0
        );

        Vk::BufferBarrierInfo sortBarrier{};
        sortBarrier.buffer = keysHandle;
        sortBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortBarrier);
    }
}