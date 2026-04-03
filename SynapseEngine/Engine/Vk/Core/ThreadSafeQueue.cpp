#include "ThreadSafeQueue.h"

namespace Syn::Vk {
	ThreadSafeQueue::ThreadSafeQueue(VkQueue handle, uint32_t familyIndex)
		: _handle(handle), _familyIndex(familyIndex) {
	}

	void ThreadSafeQueue::Submit(VkSubmitInfo2* submitInfo, VkFence fence) {
		std::scoped_lock lock(_mtx);
		SYN_VK_ASSERT(vkQueueSubmit2(_handle, 1, submitInfo, fence));
	}
}