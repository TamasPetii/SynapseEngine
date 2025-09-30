#include "ArrayIndexedManager.h"

uint32_t ArrayIndexedManager::GetAvailableIndex()
{
	std::lock_guard<std::mutex> lock(availableIndexMutex);

	if (availableIndices.empty())
		return counter++;

	uint32_t index = *availableIndices.begin();
	availableIndices.erase(index);
	return index;
}

uint32_t ArrayIndexedManager::GetCurrentCount()
{
	std::lock_guard<std::mutex> lock(availableIndexMutex);

	return counter;
}
