#include "DescriptorSet.h"

VkDescriptorSetLayoutBinding Vk::DesciptorSetUtils::BuildLayoutBindingInfo(uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, uint32_t descriptorCount)
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = type;
	layoutBinding.descriptorCount = descriptorCount;
	layoutBinding.stageFlags = flags;
	layoutBinding.pImmutableSamplers = nullptr;

	return layoutBinding;
}

VkWriteDescriptorSet Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(const DescriptorLayoutImage& imageLayout, const VkDescriptorImageInfo& imageInfo)
{
	VkWriteDescriptorSet writeInfo{};
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.dstBinding = imageLayout.config.binding;
	writeInfo.descriptorType = imageLayout.config.type;
	writeInfo.dstArrayElement = 0;
	writeInfo.descriptorCount = 1;
	writeInfo.pImageInfo = &imageInfo;

	return writeInfo;
}

Vk::DescriptorSet::DescriptorSet(VkDescriptorPool pool, const std::unordered_map<std::string, DescriptorLayoutBuffer>& bufferLayouts, const std::unordered_map<std::string, DescriptorLayoutImage>& imageLayouts) :
	pool(pool),
	bufferLayouts(bufferLayouts),
	imageLayouts(imageLayouts)
{
	Initialize();
}

Vk::DescriptorSet::~DescriptorSet()
{
	Cleanup();
}

void Vk::DescriptorSet::Free()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	if (descriptorSet != VK_NULL_HANDLE)
	{
		vkFreeDescriptorSets(device->Value(), pool, 1, &descriptorSet);
		descriptorSet = VK_NULL_HANDLE;
		Cleanup();
	}
}

const VkDescriptorSet& Vk::DescriptorSet::Value() const
{
	return descriptorSet;
}

const VkDescriptorSetLayout& Vk::DescriptorSet::Layout() const
{
	return descriptorlayout;
}

const VkDescriptorSetLayout& Vk::PushDescriptorSet::Layout() const
{
	return descriptorlayout;
}

void Vk::DescriptorSet::UpdateImageArrayElement(const std::string& name, VkImageView imageView, VkSampler sampler, uint32_t index)
{
	if (imageLayouts.find(name) == imageLayouts.end())
		return;

	auto device = VulkanContext::GetContext()->GetDevice();

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageView = imageView;
	imageInfo.sampler = sampler;
	imageInfo.imageLayout = imageLayouts[name].layout;

	VkWriteDescriptorSet writeInfo{};
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.dstSet = descriptorSet;
	writeInfo.dstBinding = imageLayouts[name].config.binding;
	writeInfo.dstArrayElement = index;
	writeInfo.descriptorType = imageLayouts[name].config.type;
	writeInfo.descriptorCount = 1;
	writeInfo.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device->Value(), 1, &writeInfo, 0, nullptr);
}

void Vk::DescriptorSet::Initialize()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (auto& [name, bufferLayout] : bufferLayouts)
	{
		VkDescriptorSetLayoutBinding layoutBinding = DesciptorSetUtils::BuildLayoutBindingInfo(bufferLayout.config.binding, bufferLayout.config.type, bufferLayout.config.flags, bufferLayout.config.descriptorCount);
		layoutBindings.push_back(layoutBinding);
	}

	std::vector<VkDescriptorBindingFlags> diDescriptorFlags;
	std::vector<uint32_t> diDescriptorSizes;

	for (auto& [name, imageLayout] : imageLayouts)
	{
		VkDescriptorSetLayoutBinding layoutBinding = DesciptorSetUtils::BuildLayoutBindingInfo(imageLayout.config.binding, imageLayout.config.type, imageLayout.config.flags, imageLayout.config.descriptorCount);
		layoutBindings.push_back(layoutBinding);

		if (imageLayout.config.descriptorCount > 1)
		{
			static VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
			diDescriptorFlags.push_back(flags);
			diDescriptorSizes.push_back(imageLayout.config.descriptorCount);
		}
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
	if (diDescriptorFlags.size() > 0)
	{
		bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlagsInfo.bindingCount = static_cast<uint32_t>(diDescriptorFlags.size());
		bindingFlagsInfo.pBindingFlags = diDescriptorFlags.data();
		layoutInfo.pNext = &bindingFlagsInfo;
	}

	VK_CHECK_MESSAGE(vkCreateDescriptorSetLayout(device->Value(), &layoutInfo, nullptr, &descriptorlayout), "Failed to create descriptor set layout!");

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorlayout;

	VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
	if (diDescriptorSizes.size() > 0)
	{
		variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
		variableCountInfo.descriptorSetCount = 1;
		variableCountInfo.pDescriptorCounts = diDescriptorSizes.data();
		allocInfo.pNext = &variableCountInfo;
	}

	VK_CHECK_MESSAGE(vkAllocateDescriptorSets(device->Value(), &allocInfo, &descriptorSet), "Failed to allocate descriptor sets!");

	std::vector<VkWriteDescriptorSet> descriptorWrites{};
	std::vector<VkDescriptorBufferInfo> bufferInfos{};
	bufferInfos.reserve(bufferLayouts.size());
	std::vector<VkDescriptorImageInfo> imageInfos{};
	imageInfos.reserve(imageLayouts.size());

	for (auto& [name, bufferLayout] : bufferLayouts)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = bufferLayout.buffer;
		bufferInfo.offset = bufferLayout.offset;
		bufferInfo.range = bufferLayout.range;
		bufferInfos.push_back(bufferInfo);

		VkWriteDescriptorSet writeInfo{};
		writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.dstSet = descriptorSet;
		writeInfo.dstBinding = bufferLayout.config.binding;
		writeInfo.dstArrayElement = 0;
		writeInfo.descriptorType = bufferLayout.config.type;
		writeInfo.descriptorCount = 1;
		writeInfo.pBufferInfo = &bufferInfos.back();

		descriptorWrites.push_back(writeInfo);
	}

	for (auto& [name, imageLayout] : imageLayouts)
	{
		if (imageLayout.config.descriptorCount > 1)
			continue;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = imageLayout.image;
		imageInfo.sampler = imageLayout.sampler;
		imageInfo.imageLayout = imageLayout.layout;
		imageInfos.push_back(imageInfo);

		VkWriteDescriptorSet writeInfo{};
		writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.dstSet = descriptorSet;
		writeInfo.dstBinding = imageLayout.config.binding;
		writeInfo.dstArrayElement = 0;
		writeInfo.descriptorType = imageLayout.config.type;
		writeInfo.descriptorCount = imageLayout.config.descriptorCount;
		writeInfo.pImageInfo = &imageInfos.back();

		descriptorWrites.push_back(writeInfo);
	}

	vkUpdateDescriptorSets(device->Value(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Vk::DescriptorSet::Cleanup()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	if (descriptorlayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device->Value(), descriptorlayout, nullptr);
		descriptorlayout = VK_NULL_HANDLE;
	}
}

Vk::DescriptorSetBuilder& Vk::DescriptorSetBuilder::AddDescriptorLayoutBuffer(const std::string& name, uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
	DescriptorLayoutBuffer bufferLayout{};
	bufferLayout.config.binding = binding;
	bufferLayout.config.type = type;
	bufferLayout.config.flags = flags;
	bufferLayout.buffer = buffer;
	bufferLayout.offset = offset;
	bufferLayout.range = range;

	bufferLayouts[name] = bufferLayout;

	return *this;
}

Vk::DescriptorSetBuilder& Vk::DescriptorSetBuilder::AddDescriptorLayoutImage(const std::string& name, uint32_t binding, VkDescriptorType type, VkShaderStageFlags flags, VkImageView image, VkSampler sampler, VkImageLayout layout, uint32_t descriptorCount)
{
	DescriptorLayoutImage imageLayout{};
	imageLayout.config.binding = binding;
	imageLayout.config.type = type;
	imageLayout.config.flags = flags;
	imageLayout.config.descriptorCount = descriptorCount;
	imageLayout.image = image;
	imageLayout.sampler = sampler;
	imageLayout.layout = layout;

	imageLayouts[name] = imageLayout;

	return *this;
}

std::shared_ptr<Vk::DescriptorSet> Vk::DescriptorSetBuilder::BuildDescriptorSet(VkDescriptorPool pool)
{
	return std::make_shared<Vk::DescriptorSet>(pool, bufferLayouts, imageLayouts);
}

std::shared_ptr<Vk::PushDescriptorSet> Vk::DescriptorSetBuilder::BuildPushDescriptorSet()
{
	return std::make_shared<Vk::PushDescriptorSet>(bufferLayouts, imageLayouts);
}

Vk::PushDescriptorSet::PushDescriptorSet(const std::unordered_map<std::string, DescriptorLayoutBuffer>& bufferLayouts, const std::unordered_map<std::string, DescriptorLayoutImage>& imageLayouts) : 
	bufferLayouts(bufferLayouts),
	imageLayouts(imageLayouts)
{
	auto device = VulkanContext::GetContext()->GetDevice();

	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (auto& [name, bufferLayout] : bufferLayouts)
	{
		VkDescriptorSetLayoutBinding layoutBinding = DesciptorSetUtils::BuildLayoutBindingInfo(bufferLayout.config.binding, bufferLayout.config.type, bufferLayout.config.flags, bufferLayout.config.descriptorCount);
		layoutBindings.push_back(layoutBinding);
	}

	for (auto& [name, imageLayout] : imageLayouts)
	{
		VkDescriptorSetLayoutBinding layoutBinding = DesciptorSetUtils::BuildLayoutBindingInfo(imageLayout.config.binding, imageLayout.config.type, imageLayout.config.flags, imageLayout.config.descriptorCount);
		layoutBindings.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

	VK_CHECK_MESSAGE(vkCreateDescriptorSetLayout(device->Value(), &layoutInfo, nullptr, &descriptorlayout), "Failed to create push descriptor set layout!");
}

Vk::PushDescriptorSet::~PushDescriptorSet()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	if (descriptorlayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(device->Value(), descriptorlayout, nullptr);
		descriptorlayout = VK_NULL_HANDLE;
	}
}
