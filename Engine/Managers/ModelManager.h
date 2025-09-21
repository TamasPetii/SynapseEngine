#pragma once
#include <memory>
#include <set>
#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "Engine/Renderable/Model/Model.h"
#include "BaseManagers/DeviceAddressedManager.h"
#include "BaseManagers/AsyncManager.h"

struct ENGINE_API ModelDevicesAddresses
{
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	VkDeviceAddress materialBufferAddress;
	VkDeviceAddress nodeTransformBufferAddress;
};

class ENGINE_API ModelManager : public DeviceAddressedManager<ModelDevicesAddresses>, public AsyncManager<std::string>
{
public:
	ModelManager(std::shared_ptr<ImageManager> imageManager, std::shared_ptr<MaterialManager> materialManager);
	~ModelManager();
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	std::shared_ptr<Model> LoadModel(const std::string& path);
	std::shared_ptr<Model> GetModel(const std::string& path);
	const auto& GetModels() { return models; }
	void Update();
private:
	std::shared_ptr<ImageManager> imageManager = nullptr;
	std::shared_ptr<MaterialManager> materialManager = nullptr;
	std::unordered_map<std::string, std::shared_ptr<Model>> models;
};

