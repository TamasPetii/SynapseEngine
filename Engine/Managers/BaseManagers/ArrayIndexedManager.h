#pragma once
#include "Engine/EngineApi.h"
#include <set>
#include <mutex>
#include <atomic>

class ENGINE_API ArrayIndexedManager
{
protected:
	uint32_t GetAvailableIndex();
	uint32_t GetCurrentCount();
protected:
	std::mutex availableIndexMutex;
	std::atomic<uint32_t> counter = 0;
	std::set<uint32_t> availableIndices;
};