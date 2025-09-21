#pragma once
#include "Engine/EngineApi.h"
#include "Engine/Vulkan/Image.h"
#include "Engine/Utils/AsyncLoaded.h"
#include "Engine/Utils/BatchUploaded.h"
#include "Engine/Utils/BufferArrayIndexed.h"
#include <memory>
#include <string>

class ENGINE_API ImageTexture : public AsyncLoaded, public BatchUploaded, public BufferArrayIndexed
{
public:
	ImageTexture();
	void Load(const std::string& path, bool generateMipMap);
	auto& GetImage() { return image; }
	const auto& GetPath() { return path; }
private:
	void CreateVulkanImageWithGli(const std::string& path);
	void CreateVulkanImageWithStb(const std::string& path, bool generateMipMap);
private:
	std::string path;
	std::unique_ptr<Vk::Image> image;
};

