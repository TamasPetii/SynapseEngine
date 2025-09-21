#include "ImageTexture.h"
#include <stb_image.h>
#include <gli/gli.hpp>

#include <filesystem>

#include "Engine/Vulkan/Buffer.h"
#include "Engine/Vulkan/VulkanContext.h"

VkFormat GliFormatToVulkan(gli::format format)
{
	static const std::unordered_map<gli::format, VkFormat> formatMap = {
		// BC (DXT) compressed formats
		{gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, VK_FORMAT_BC1_RGB_UNORM_BLOCK},
		{gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, VK_FORMAT_BC1_RGBA_UNORM_BLOCK},
		{gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16, VK_FORMAT_BC2_UNORM_BLOCK},
		{gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, VK_FORMAT_BC3_UNORM_BLOCK},
		{gli::FORMAT_R_ATI1N_UNORM_BLOCK8, VK_FORMAT_BC4_UNORM_BLOCK},
		{gli::FORMAT_RG_ATI2N_UNORM_BLOCK16, VK_FORMAT_BC5_UNORM_BLOCK},
		{gli::FORMAT_RGBA_BP_UNORM_BLOCK16, VK_FORMAT_BC7_UNORM_BLOCK},

		// Uncompressed formats
		{gli::FORMAT_RGBA8_UNORM_PACK8, VK_FORMAT_R8G8B8A8_UNORM},
		{gli::FORMAT_RGB8_UNORM_PACK8, VK_FORMAT_R8G8B8_UNORM},
		{gli::FORMAT_R8_UNORM_PACK8, VK_FORMAT_R8_UNORM},
		{gli::FORMAT_RG8_UNORM_PACK8, VK_FORMAT_R8G8_UNORM},
		{gli::FORMAT_RGBA16_UNORM_PACK16, VK_FORMAT_R16G16B16A16_UNORM},
		{gli::FORMAT_RGB16_UNORM_PACK16, VK_FORMAT_R16G16B16_UNORM},
		{gli::FORMAT_R16_UNORM_PACK16, VK_FORMAT_R16_UNORM},

		// SRGB formats
		{gli::FORMAT_RGB_DXT1_SRGB_BLOCK8, VK_FORMAT_BC1_RGB_SRGB_BLOCK},
		{gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8, VK_FORMAT_BC1_RGBA_SRGB_BLOCK},
		{gli::FORMAT_RGBA_DXT3_SRGB_BLOCK16, VK_FORMAT_BC2_SRGB_BLOCK},
		{gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16, VK_FORMAT_BC3_SRGB_BLOCK},
		{gli::FORMAT_RGBA_BP_SRGB_BLOCK16, VK_FORMAT_BC7_SRGB_BLOCK},
		{gli::FORMAT_RGBA8_SRGB_PACK8, VK_FORMAT_R8G8B8A8_SRGB},
		{gli::FORMAT_RGB8_SRGB_PACK8, VK_FORMAT_R8G8B8_SRGB},

		// Floating-point formats
		{gli::FORMAT_RGBA16_SFLOAT_PACK16, VK_FORMAT_R16G16B16A16_SFLOAT},
		{gli::FORMAT_RGB16_SFLOAT_PACK16, VK_FORMAT_R16G16B16_SFLOAT},
		{gli::FORMAT_R16_SFLOAT_PACK16, VK_FORMAT_R16_SFLOAT},
		{gli::FORMAT_RGBA32_SFLOAT_PACK32, VK_FORMAT_R32G32B32A32_SFLOAT},
		{gli::FORMAT_RGB32_SFLOAT_PACK32, VK_FORMAT_R32G32B32_SFLOAT},
		{gli::FORMAT_R32_SFLOAT_PACK32, VK_FORMAT_R32_SFLOAT},
	};

	auto it = formatMap.find(format);
	if (it != formatMap.end()) {
		return it->second;
	}

	return VK_FORMAT_UNDEFINED;
}


ImageTexture::ImageTexture()
{
}

void ImageTexture::Load(const std::string& path, bool generateMipMap)
{
	this->path = path;

	std::string extension = std::filesystem::path(path).extension().string();
	if (extension == ".dds")
		CreateVulkanImageWithGli(path);
	else
		CreateVulkanImageWithStb(path, generateMipMap);

	state = LoadState::CpuLoaded;
}

void ImageTexture::CreateVulkanImageWithGli(const std::string& path)
{
	gli::texture texture = gli::load(path);

	if (texture.empty() || texture.target() != gli::target::TARGET_2D || texture.layers() != 1)
	{
		state = LoadState::Failed;
		image = nullptr;
		return;
	}

	uint32_t texSize = texture.size();
	uint32_t texWidth = texture.extent().x;
	uint32_t texHeight = texture.extent().y;
	uint32_t mipMapLevel = texture.levels();
	VkFormat format = GliFormatToVulkan(texture.format());

	Vk::ImageSpecification imageSpec;
	imageSpec.width = texWidth;
	imageSpec.height = texHeight;
	imageSpec.type = VK_IMAGE_TYPE_2D;
	imageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageSpec.format = format;
	imageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageSpec.mipmapLevel = mipMapLevel;
	image = std::make_unique<Vk::Image>(imageSpec);

	VkDeviceSize totalSize = 0;
	std::vector<VkDeviceSize> offsets;
	for (uint32_t level = 0; level < imageSpec.mipmapLevel; ++level) {
		offsets.push_back(totalSize);
		totalSize += texture.size(level);
	}

	Vk::BufferConfig stagingConfig;
	stagingConfig.size = totalSize;
	stagingConfig.usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT;
	stagingConfig.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	uploadData.stagingBuffer = std::make_shared<Vk::Buffer>(stagingConfig);
	void* mappedBuffer = uploadData.stagingBuffer->MapMemory();

	VkDeviceSize offset = 0;
	for (uint32_t level = 0; level < imageSpec.mipmapLevel; ++level)
	{
		uint32_t mipWidth = texture.extent(level).x;
		uint32_t mipHeight = texture.extent(level).y;
		VkDeviceSize mipSize = texture.size(level);
		void* data = texture.data(0, 0, level);
		memcpy(static_cast<char*>(mappedBuffer) + offset, data, (size_t)mipSize);
		offset += mipSize;
	}

	uploadData.stagingBuffer->UnmapMemory();

	BatchUploaded::CreateCommandPoolAndBuffer();
	BatchUploaded::BeginCommandBuffer();

	for (uint32_t level = 0; level < imageSpec.mipmapLevel; ++level)
	{
		uint32_t mipWidth = texture.extent(level).x;
		uint32_t mipHeight = texture.extent(level).y;

		Vk::Image::TransitionImageLayoutDynamic(uploadData.commandBuffer, image->Value(),
			VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, 1, level);

		Vk::Buffer::CopyBufferToImage(uploadData.commandBuffer, uploadData.stagingBuffer->Value(), image->Value(), mipWidth, mipHeight, level, offsets[level]);

		Vk::Image::TransitionImageLayoutDynamic(uploadData.commandBuffer, image->Value(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, 1, level);
	}

	BatchUploaded::EndCommandBuffer();
}

void ImageTexture::CreateVulkanImageWithStb(const std::string& path, bool generateMipMap)
{
	int width, height, channels;
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!data)
	{
		state = LoadState::Failed;
		image = nullptr;
		return;
	}

	uint32_t texWidth = static_cast<uint32_t>(width);
	uint32_t texHeight = static_cast<uint32_t>(height);
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	VkDeviceSize size = texWidth * texHeight * 4;
	uint32_t mipMapLevel = 1;

	if (generateMipMap && Vk::Image::ImageFormatSupportsLinearMipMap(format))
		mipMapLevel = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

	Vk::BufferConfig stagingConfig;
	stagingConfig.size = size;
	stagingConfig.usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT;
	stagingConfig.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	uploadData.stagingBuffer = std::make_shared<Vk::Buffer>(stagingConfig);

	void* mappedBuffer = uploadData.stagingBuffer->MapMemory();
	memcpy(mappedBuffer, data, (size_t)size);
	uploadData.stagingBuffer->UnmapMemory();

	stbi_image_free(data);

	Vk::ImageSpecification imageSpec;
	imageSpec.width = texWidth;
	imageSpec.height = texHeight;
	imageSpec.type = VK_IMAGE_TYPE_2D;
	imageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageSpec.format = format;
	imageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	imageSpec.mipmapLevel = mipMapLevel;
	image = std::make_unique<Vk::Image>(imageSpec);

	BatchUploaded::CreateCommandPoolAndBuffer();
	BatchUploaded::BeginCommandBuffer();

	//Transition all image mipmap layers at once
	Vk::Image::TransitionImageLayoutDynamic(uploadData.commandBuffer, image->Value(),
		VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, imageSpec.mipmapLevel);

	Vk::Buffer::CopyBufferToImage(uploadData.commandBuffer, uploadData.stagingBuffer->Value(), image->Value(), imageSpec.width, imageSpec.height);

	if (imageSpec.mipmapLevel > 1)
	{
		Vk::Image::GenerateMipMaps(uploadData.commandBuffer, image->Value(), imageSpec.width, imageSpec.height, imageSpec.mipmapLevel);
	}
	else
	{
		Vk::Image::TransitionImageLayoutDynamic(uploadData.commandBuffer, image->Value(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
	}

	BatchUploaded::EndCommandBuffer();
}
