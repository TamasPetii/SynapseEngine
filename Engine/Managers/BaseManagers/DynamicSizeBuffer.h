#pragma once
#include <memory>
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Components/BaseComponents/VersionIndexed.h"

struct DynamicSizeBuffer : public VersionIndexed
{
	uint32_t size = 0;
	std::shared_ptr<Vk::Buffer> buffer = nullptr;
};