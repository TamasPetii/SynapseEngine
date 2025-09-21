#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "Engine/Managers/VulkanManager.h"
#include "BaseManagers/DeviceAddressedManager.h"
#include "Engine/Utils/Material.h"
#include "Engine/Registry/Pools/ComponentPool.h"
#include "ImageManager.h"

//Todo: Make full dynamic material manager with component pool???

class ENGINE_API MaterialManager : public DeviceAddressedManager<MaterialGPU>
{
public:
	MaterialManager(std::shared_ptr<ImageManager> imageManager);
	void Update();
	std::pair<std::shared_ptr<Material>, bool> RegisterMaterial(const std::string& name);
	const auto& GetMaterial(const std::string& name) { return materials.at(name); } //TODO
private:
	bool IsMaterialLoaded(const std::string& name) { return materials.find(name) != materials.end(); }
private:
	std::mutex asyncMutex;
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;
};

