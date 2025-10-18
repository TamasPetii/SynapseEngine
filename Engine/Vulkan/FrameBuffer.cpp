#include "FrameBuffer.h"

Vk::FrameBuffer::FrameBuffer(FrameBufferConfig& config) :
	config(config)
{
	Initialize();
}

Vk::FrameBuffer::~FrameBuffer()
{
	Cleanup();
}

void Vk::FrameBuffer::Resize(uint32_t width, uint32_t height)
{
	config.width = width;
	config.height = height;
	Cleanup();
	Initialize();
}

VkFramebuffer Vk::FrameBuffer::GetFrameBuffer() const
{
	return frameBuffer;
}

VkExtent2D Vk::FrameBuffer::GetSize() const
{
	return VkExtent2D(config.width, config.height);
}

std::shared_ptr<Vk::Image> Vk::FrameBuffer::GetImage(const std::string& imageName) const
{
	auto& imageData = config.imageSpecifications.at(imageName);
	return images[imageData.first];
}

bool Vk::FrameBuffer::Initialize()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	images.resize(config.imageSpecifications.size());

	for (auto& [imageName, imageData] : config.imageSpecifications)
	{
		imageData.second.width = config.width;
		imageData.second.height = config.height;
		images[imageData.first] = std::make_shared<Image>(imageData.second);
	}

	return true;
}

void Vk::FrameBuffer::Cleanup()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	images.clear();

	if (frameBuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device->Value(), frameBuffer, nullptr);
		frameBuffer = VK_NULL_HANDLE;
	}
}


Vk::FrameBufferBuilder::FrameBufferBuilder()
{
	ResetToDefault();
}

void Vk::FrameBufferBuilder::ResetToDefault()
{
	config = FrameBufferConfig{};
}

Vk::FrameBufferBuilder& Vk::FrameBufferBuilder::SetSize(uint32_t width, uint32_t height)
{
	config.width = width;
	config.height = height;
	return *this;
}

Vk::FrameBufferBuilder& Vk::FrameBufferBuilder::AddImageSpecification(const std::string& imageName,	uint32_t index,	const ImageSpecification& specification)
{
	config.imageSpecifications[imageName] = { index, specification };
	return *this;
}

Vk::FrameBufferBuilder& Vk::FrameBufferBuilder::AddDepthSpecification(uint32_t index, const ImageSpecification& specification)
{
	return AddImageSpecification("Depth", index, specification);
}

std::shared_ptr<Vk::FrameBuffer> Vk::FrameBufferBuilder::Build(VkRenderPass renderPass)
{
	config.renderPass = renderPass;
	return std::make_shared<FrameBuffer>(config);
}

std::shared_ptr<Vk::FrameBuffer> Vk::FrameBufferBuilder::BuildDynamic()
{
	return std::make_shared<FrameBuffer>(config);
}