#pragma once
#include "Engine/EngineApi.h"
#include <memory>
#include <string>

class ENGINE_API BufferArrayIndexed
{
public:
	const auto& GetBufferArrayIndex() const { return bufferArrayIndex; }
	void SetBufferArrayIndex(uint32_t index) { bufferArrayIndex = index; }
protected:
	uint32_t bufferArrayIndex = UINT32_MAX;
};

