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

#include "TimelineSemaphore.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {
    TimelineSemaphore::TimelineSemaphore(uint64_t initialValue) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();

        VkSemaphoreTypeCreateInfo typeInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
        typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        typeInfo.initialValue = initialValue;

        VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        info.pNext = &typeInfo;

        SYN_VK_ASSERT_MSG(vkCreateSemaphore(device->Handle(), &info, nullptr, &_handle), "Failed to create Timeline Semaphore");
    }

    void TimelineSemaphore::Signal(uint64_t value) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        VkSemaphoreSignalInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO };
        info.semaphore = _handle;
        info.value = value;

        SYN_VK_ASSERT_MSG(vkSignalSemaphore(device->Handle(), &info), "Failed to signal Timeline Semaphore");
    }

    void TimelineSemaphore::Wait(uint64_t value, uint64_t timeout) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        VkSemaphoreWaitInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
        info.semaphoreCount = 1;
        info.pSemaphores = &_handle;
        info.pValues = &value;

        SYN_VK_ASSERT_MSG(vkWaitSemaphores(device->Handle(), &info, timeout), "Failed to wait for Timeline Semaphore");
    }

    uint64_t TimelineSemaphore::GetValue() const {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        uint64_t value = 0;
        SYN_VK_ASSERT_MSG(vkGetSemaphoreCounterValue(device->Handle(), _handle, &value), "Failed to get Timeline Semaphore value");
        return value;
    }

}