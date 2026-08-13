// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "SpotLightShadowAtlasRadixSortPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Context.h"

#include <volk.h>
#include <vk_radix_sort.h>

namespace Syn {
    SpotLightShadowAtlasRadixSortPass::~SpotLightShadowAtlasRadixSortPass() {
        if (_radixSorter != VK_NULL_HANDLE) {
            vrdxDestroySorter(_radixSorter);
            _radixSorter = VK_NULL_HANDLE;
        }
    }

    void SpotLightShadowAtlasRadixSortPass::Initialize() {
        auto vulkanContext = ServiceLocator::Get<Vk::Context>();

        VrdxSorterCreateInfo sorterInfo = {};
        sorterInfo.physicalDevice = vulkanContext->GetPhysicalDevice()->Handle();
        sorterInfo.device = vulkanContext->GetDevice()->Handle();
        vrdxCreateSorter(&sorterInfo, &_radixSorter);
    }

    bool SpotLightShadowAtlasRadixSortPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return context.scene->GetSettings()->culling.spotLightCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void SpotLightShadowAtlasRadixSortPass::Execute(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        uint32_t maxSortCount = static_cast<uint32_t>(pool->Size());
        if (maxSortCount == 0) return;

        auto& tempBuffer = drawData->SpotLightShadow.atlasRadixSortTempBuffer;

        VrdxSorterStorageRequirements reqs;
        vrdxGetSorterStorageRequirements(_radixSorter, maxSortCount, VRDX_SORT_MODE_KEY_VALUE, &reqs);
        tempBuffer.UpdateCapacity(fIdx, reqs.size);

        VkBuffer keysHandle = compManager->GetComponentBuffer(BufferNames::SpotLightShadowAtlasSortKeyBuffer, fIdx).buffer->Handle();
        VkBuffer valuesHandle = compManager->GetComponentBuffer(BufferNames::SpotLightShadowAtlasSortValueBuffer, fIdx).buffer->Handle();
        VkBuffer countBuffer = drawData->SpotLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = countBuffer;
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
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

        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = keysHandle;
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);

        Vk::BufferBarrierInfo valuesBarrier{};
        valuesBarrier.buffer = valuesHandle;
        valuesBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesBarrier);
    }
}