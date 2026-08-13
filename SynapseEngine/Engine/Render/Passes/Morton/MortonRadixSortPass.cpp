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

#include "MortonRadixSortPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

#include <volk.h>
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
        auto vulkanContext = ServiceLocator::Get<Vk::Context>();
        VkPhysicalDevice physicalDevice = vulkanContext->GetPhysicalDevice()->Handle();
        VkDevice device = vulkanContext->GetDevice()->Handle();

        VrdxSorterCreateInfo sorterInfo = {};
        sorterInfo.physicalDevice = physicalDevice;
        sorterInfo.device = device;
        vrdxCreateSorter(&sorterInfo, &_radixSorter);
    }

    bool MortonRadixSortPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();

        bool isEnabled = context.scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh;

        if (!isEnabled || !pool || pool->GetStorage().GetStaticEntities().empty()) {
            _wasEnabled = false;
            return false;
        }

        if (!_wasEnabled) {
            _needsRebuild = true;
        }
        _wasEnabled = true;

        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();
        return hasDirty || _needsRebuild || (_countdown > 0);
    }

    void MortonRadixSortPass::Execute(const RenderContext& context) {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();

        if (hasDirty || _needsRebuild) {
            _countdown = context.framesInFlight;
            _needsRebuild = false;
        }

        if (_countdown > 0) {
            _countdown--;
        }

        _staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
        if (_staticCount == 0) return;

        auto drawData = context.scene->GetSceneDrawData();
        auto compManager = context.scene->GetComponentBufferManager();
        auto& tempBuffer = drawData->Chunks.mortonRadixSortTempBuffer;
        uint32_t fIdx = context.frameIndex;

        VrdxSorterStorageRequirements reqs;
        vrdxGetSorterStorageRequirements(_radixSorter, _staticCount, VRDX_SORT_MODE_KEY_VALUE, &reqs);
        tempBuffer.UpdateCapacity(context.frameIndex, reqs.size);

        VkBuffer keysHandle = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        VkBuffer valuesHandle = compManager->GetComponentBuffer(BufferNames::MortonValuesData, fIdx).buffer->Handle();
        VkBuffer tempHandle = tempBuffer.GetHandle(context.frameIndex);

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
        sortInfo.elementCount = _staticCount;
        sortInfo.elementCountBuffer = VK_NULL_HANDLE;
        sortInfo.elementCountOffset = 0;
        sortInfo.keysBuffer = keysHandle;
        sortInfo.keysOffset = 0;
        sortInfo.valuesBuffer = valuesHandle;
        sortInfo.valuesOffset = 0;
        sortInfo.storageBuffer = tempHandle;
        sortInfo.storageOffset = 0;
        sortInfo.queryPool = VK_NULL_HANDLE;
        sortInfo.query = 0;

        vrdxCmdSort(context.cmd, _radixSorter, &sortInfo);

        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = keysHandle;
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);

        Vk::BufferBarrierInfo valuesBarrier{};
        valuesBarrier.buffer = valuesHandle;
        valuesBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        valuesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesBarrier);
    }
}