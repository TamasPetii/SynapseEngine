#pragma once
#include <memory>
#include <set>
#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "Engine/Animation/Animation.h"
#include "BaseManagers/DeviceAddressedManager.h"
#include "BaseManagers/AsyncManager.h"
#include "Engine/Utils/VersionedObject.h"

class ENGINE_API AnimationManager : public DeviceAddressedManager<VkDeviceAddress>, public AsyncManager<std::string>
{
public:
	AnimationManager();
	~AnimationManager();
	AnimationManager(const AnimationManager&) = delete;
	AnimationManager& operator=(const AnimationManager&) = delete;

	void Update(uint32_t frameIndex);
	std::shared_ptr<Animation> LoadAnimation(const std::string& path);
	std::shared_ptr<Animation> GetAnimation(const std::string& path);
	const auto& GetAnimations() { return animations; }
private:
	std::unordered_map<std::string, std::shared_ptr<VersionedObject<Animation>>> animations;
};

