#pragma once
#include <array>
#include <glm/glm.hpp>
#include "Engine/Config.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Utils/Material.h"

class MaterialManager;

class ENGINE_API Materialized
{
public:
	std::shared_ptr<Vk::Buffer> GetMaterialBuffer();
	virtual void UploadMaterialDataToGpu(std::shared_ptr<MaterialManager> materialManager);
protected:
	std::vector<std::shared_ptr<Material>> materials;
	std::shared_ptr<Vk::Buffer> materialBuffer;
};

