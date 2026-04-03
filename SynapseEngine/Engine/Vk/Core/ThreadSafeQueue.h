#pragma once
#include "../VkCommon.h"
#include <mutex>

namespace Syn::Vk 
{
    class SYN_API ThreadSafeQueue 
    {
    public:
        ThreadSafeQueue(VkQueue handle, uint32_t familyIndex);

        void Submit(VkSubmitInfo2* submitInfo, VkFence fence);
        VkQueue Handle() const { return _handle; }
        uint32_t GetFamilyIndex() const { return _familyIndex; }
    private:
        VkQueue _handle = VK_NULL_HANDLE;
        uint32_t _familyIndex = 0;
        std::mutex _mtx;
    };
}