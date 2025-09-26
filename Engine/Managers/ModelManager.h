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
#include "BaseManagers/DrawIndirectManager.h"
#include "Engine/Utils/VersionedObject.h"

struct ENGINE_API ModelDeviceAddresses
{
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	VkDeviceAddress materialBufferAddress;
	VkDeviceAddress nodeTransformBufferAddress;
};

class ENGINE_API ModelManager : public DeviceAddressedManager<ModelDeviceAddresses>, public AsyncManager<std::string>, public DrawIndirectManager
{
public:
	ModelManager(std::shared_ptr<ImageManager> imageManager, std::shared_ptr<MaterialManager> materialManager);
	~ModelManager();
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	std::shared_ptr<Model> LoadModel(const std::string& path);
	std::shared_ptr<Model> GetModel(const std::string& path);
	const auto& GetModels() { return models; }
	void Update(uint32_t frameIndex);
private:
	std::shared_ptr<ImageManager> imageManager = nullptr;
	std::shared_ptr<MaterialManager> materialManager = nullptr;
	std::unordered_map<std::string, std::shared_ptr<VersionedObject<Model>>> models;
};

