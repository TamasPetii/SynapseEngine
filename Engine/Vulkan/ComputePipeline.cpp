#include "ComputePipeline.h"

Vk::ComputePipeline::ComputePipeline(const ComputePipelineConfig& config)
{
	auto device = VulkanContext::GetContext()->GetDevice();

	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.setLayoutCount = static_cast<uint32_t>(config.descriptorLayouts.size());
	layoutInfo.pSetLayouts = config.descriptorLayouts.data();
	layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(config.pushConstants.size());
	layoutInfo.pPushConstantRanges = config.pushConstants.data();

	VK_CHECK_MESSAGE(vkCreatePipelineLayout(device->Value(), &layoutInfo, nullptr, &layout), "Failed to create pipeline layout");

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = layout;
	computePipelineCreateInfo.stage = config.shaderStage;

	VK_CHECK_MESSAGE(vkCreateComputePipelines(device->Value(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &pipeline), "Failed to create graphics pipeline");
}

Vk::ComputePipeline::~ComputePipeline()
{
	auto device = VulkanContext::GetContext()->GetDevice();

	if (pipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(device->Value(), pipeline, nullptr);
		pipeline = VK_NULL_HANDLE;
	}
	if (layout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(device->Value(), layout, nullptr);
		layout = VK_NULL_HANDLE;
	}
}

Vk::ComputePipelineBuilder& Vk::ComputePipelineBuilder::AddShaderStage(std::shared_ptr<ShaderModule> shader)
{
	config.shaderStage = shader->GetPipelineShaderStageInfo();
	return *this;
}

Vk::ComputePipelineBuilder& Vk::ComputePipelineBuilder::AddPushConstant(uint32_t offset, uint32_t size, VkShaderStageFlags stages)
{
	VkPushConstantRange pushConstant{};
	pushConstant.offset = offset;
	pushConstant.size = size;
	pushConstant.stageFlags = stages;
	config.pushConstants.push_back(pushConstant);
	return *this;
}

Vk::ComputePipelineBuilder& Vk::ComputePipelineBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout layout)
{
	config.descriptorLayouts.push_back(layout);
	return *this;
}

std::shared_ptr<Vk::ComputePipeline> Vk::ComputePipelineBuilder::BuildDynamic()
{
	return std::make_shared<ComputePipeline>(config);
}