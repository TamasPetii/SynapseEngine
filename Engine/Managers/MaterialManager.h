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
#include "Engine/Utils/VersionedObject.h"

//Todo: Make full dynamic material manager with component pool???

/* Bug Alert
What if multiple materials use the same texture? Only the first one processing the texture will get called by onFinish()???
So in case of other materials finished (Other textures loaded), they wont get triggered!!
*/

class ENGINE_API MaterialManager : public DeviceAddressedManager<MaterialGPU>
{
public:
	MaterialManager(std::shared_ptr<ImageManager> imageManager);
	std::pair<std::shared_ptr<Material>, bool> RegisterMaterial(const std::string& name);
	const auto& GetMaterial(const std::string& name) { return materials.at(name)->object; } //TODO

	void Update(uint32_t frameIndex);
private:
	bool IsMaterialLoaded(const std::string& name) { return materials.find(name) != materials.end(); }
private:
	std::mutex asyncMutex;
	std::unordered_map<std::string, std::shared_ptr<VersionedObject<Material>>> materials;
};

