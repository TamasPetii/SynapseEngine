#include "Instanceable.h"

void Instanceable::AddIndex(const uint32_t index)
{
	instanceIndices.push_back(index);
	instanceCount++;
}

void Instanceable::UploadInstanceDataToGPU(uint32_t maxInstanceCount, uint32_t frameIndex)
{
	if (instanceCount == 0 && instanceIndexBuffers[frameIndex].first != nullptr)
		return;

	//Upload the vertices to the gpu
	{
		constexpr uint32_t instanceBlockSize = 128;
		uint32_t requiredSize = static_cast<uint32_t>(std::ceil((maxInstanceCount + 1) / (float)instanceBlockSize)) * instanceBlockSize;

		if (instanceIndexBuffers[frameIndex].first == nullptr || instanceIndexBuffers[frameIndex].second != requiredSize)
		{
			Vk::BufferConfig config;
			config.size = requiredSize * sizeof(uint32_t);
			config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
			config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;	
			instanceIndexBuffers[frameIndex] = std::make_pair(std::make_shared<Vk::Buffer>(config), requiredSize);
			instanceIndexBuffers[frameIndex].first->MapMemory();

			std::cout << "Regenerated instace buffer" << std::endl;
		}

		VkDeviceSize bufferSize = sizeof(uint32_t) * instanceIndices.size();
		auto mappedBuffer = instanceIndexBuffers[frameIndex].first->GetHandler();
		memcpy(mappedBuffer, instanceIndices.data(), (size_t)bufferSize);
		instanceIndices.clear();
	}
}

std::shared_ptr<Vk::Buffer> Instanceable::GetInstanceIndexBuffer(uint32_t frameIndex)
{
	return instanceIndexBuffers[frameIndex].first;
}

uint32_t Instanceable::GetInstanceCount()
{
	return instanceCount;
}

void Instanceable::ResetInstanceCount()
{
	instanceCount = 0;
	instanceIndices.clear();
}

/*
* Using the std::shared_ptr counter the instanceIndices size can be estimated and reserved.
* It approches the number of max components refering to the specific ptr. 
*/
void Instanceable::ReserveInstances(uint32_t size)
{
	instanceIndices.reserve(size);
}

/*
* Shrink the reserved instanceIndices to fit the actual number of indices, needed for .data()
*/

void Instanceable::ShrinkInstances()
{
	instanceIndices.resize(instanceCount);
}
