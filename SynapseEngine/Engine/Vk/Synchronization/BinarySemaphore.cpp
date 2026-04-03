#include "BinarySemaphore.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {
    BinarySemaphore::BinarySemaphore() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();
        VkSemaphoreCreateInfo info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

        SYN_VK_ASSERT_MSG(vkCreateSemaphore(device->Handle(), &info, nullptr, &_handle), "Failed to create Binary Semaphore");
    }
}