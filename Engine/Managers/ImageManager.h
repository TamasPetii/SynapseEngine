#pragma once
#include <memory>
#include <set>
#include <string>
#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "Engine/Utils/ImageTexture.h"
#include "Engine/Managers/VulkanManager.h"
#include "BaseManagers/ArrayIndexedManager.h"
#include "BaseManagers/AsyncManager.h"
#include "BaseManagers/BatchedGpuUploadedManager.h"

enum class ImageLoadMode
{
	Sync,
	Async
};

class ENGINE_API ImageManager : public ArrayIndexedManager, public AsyncManager<std::string>, public AsyncBatchedGpuUploadedManager<ImageTexture, 32>
{
public:
	ImageManager(std::shared_ptr<VulkanManager> vulkanManager);
	~ImageManager();

	ImageManager(const ImageManager&) = delete;
	ImageManager& operator=(const ImageManager&) = delete;

	void Update();
	std::shared_ptr<ImageTexture> GetImage(const std::string& path);
	std::shared_ptr<ImageTexture> LoadImage(const std::string& path, const ImageLoadMode& mode = ImageLoadMode::Async, bool generateMipMap = true, std::function<void()> onFinished = nullptr);
private:
	std::shared_ptr<VulkanManager> vulkanManager = nullptr;
	std::unordered_map<std::string, std::shared_ptr<ImageTexture>> images;
	std::unordered_map<std::string, std::function<void()>> onFinishFunctions;
};

