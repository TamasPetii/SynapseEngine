#include "VulkanManager.h"
#include "Engine/Render/GpuStructs.h"

VulkanManager::VulkanManager()
{
	Initialize();
}

VulkanManager::~VulkanManager()
{
	Cleanup();
}

void VulkanManager::Initialize()
{
	InitSamplers();
	InitShaderModuls();
	InitRenderPasses();
	InitFrameBuffers();
	InitDescriptors();
	InitGraphicsPipelines();
	InitComputePipelines();
	InitFences();
	InitSemaphores();
}

void VulkanManager::Cleanup()
{
	descriptorPools.clear();
	samplers.clear();
	renderPasses.clear();
	shaderModuls.clear();
	graphicsPipelines.clear();
	frameDependentBuffers.clear();
	frameDependentFrameBuffers.clear();
	frameDependentDescriptorSets.clear();
}

void VulkanManager::RegisterSampler(const std::string& name, const Vk::ImageSamplerConfig& config)
{
	RegisterSampler(name, std::make_shared<Vk::ImageSampler>(config));
}

void VulkanManager::RegisterSampler(const std::string& name, std::shared_ptr<Vk::ImageSampler> sampler)
{
	samplers[name] = sampler;
}

std::shared_ptr<Vk::ImageSampler> VulkanManager::GetSampler(const std::string& name) const
{
	if (samplers.find(name) == samplers.end())
		return nullptr;

	return samplers.at(name);
}

void VulkanManager::RegisterRenderPass(const std::string& name, std::shared_ptr<Vk::RenderPass> renderPass)
{
	renderPasses[name] = renderPass;
}

std::shared_ptr<Vk::RenderPass> VulkanManager::GetRenderPass(const std::string& name) const
{
	if (renderPasses.find(name) == renderPasses.end())
		return nullptr;

	return renderPasses.at(name);
}

void VulkanManager::RegisterShaderModule(const std::string& name, std::shared_ptr<Vk::ShaderModule> shaderModule)
{
	shaderModuls[name] = shaderModule;
}

std::shared_ptr<Vk::ShaderModule> VulkanManager::GetShaderModule(const std::string& name) const
{
	if (shaderModuls.find(name) == shaderModuls.end())
		return nullptr;

	return shaderModuls.at(name);
}

void VulkanManager::RegisterGraphicsPipeline(const std::string& name, std::shared_ptr<Vk::GraphicsPipeline> graphicsPipeline)
{
	graphicsPipelines[name] = graphicsPipeline;
}

std::shared_ptr<Vk::GraphicsPipeline> VulkanManager::GetGraphicsPipeline(const std::string& name) const
{
	if (graphicsPipelines.find(name) == graphicsPipelines.end())
		return nullptr;

	return graphicsPipelines.at(name);
}

void VulkanManager::RegisterComputePipeline(const std::string& name, std::shared_ptr<Vk::ComputePipeline> computePipeline)
{
	computePipelines[name] = computePipeline;
}

std::shared_ptr<Vk::ComputePipeline> VulkanManager::GetComputePipeline(const std::string& name) const
{
	if (computePipelines.find(name) == computePipelines.end())
		return nullptr;

	return computePipelines.at(name);
}

void VulkanManager::MarkFrameBufferToResize(const std::string& name, uint32_t index, uint32_t width, uint32_t height)
{
	frameBuffersToResize.insert(std::make_tuple(name, index, width, height));
}

void VulkanManager::ResizeMarkedFrameBuffers(uint32_t frameIndex)
{
	for (auto it = frameBuffersToResize.begin(); it != frameBuffersToResize.end();)
	{
		auto& [name, index, width, height] = *it;

		if (index == frameIndex)
		{
			//Resize Framebuffer
			GetFrameDependentFrameBuffer(name, index)->Resize(width, height);

			//Regenerate descriptor for framebuffers, need some association mechanism between fbo and dsc
			if (name == "Main")
			{
				GetFrameDependentDescriptorSet("MainFrameBuffer", index)->Free();
				InitMainFramebufferDescriptorSet(index);
			}

			//std::cout << std::format("Resized framebuffer {} index {}: {} {}", name, index, width, height) << std::endl;

			it = frameBuffersToResize.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void VulkanManager::RegisterFrameDependentFrameBuffer(const std::string& name, std::shared_ptr<Vk::FrameBuffer> frameBuffer, uint32_t frameIndex)
{
	if (frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return;

	frameDependentFrameBuffers[name][frameIndex] = frameBuffer;
}

std::shared_ptr<Vk::FrameBuffer> VulkanManager::GetFrameDependentFrameBuffer(const std::string& name, uint32_t frameIndex) const
{
	if (frameDependentFrameBuffers.find(name) == frameDependentFrameBuffers.end() || frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return nullptr;

	return frameDependentFrameBuffers.at(name)[frameIndex];
}

void VulkanManager::RegisterDescriptorPool(const std::string& name, std::shared_ptr<Vk::DescriptorPool> pool)
{
	descriptorPools[name] = pool;
}

std::shared_ptr<Vk::DescriptorPool> VulkanManager::GetDescriptorPool(const std::string& name) const
{
	if (descriptorPools.find(name) == descriptorPools.end())
		return nullptr;

	return descriptorPools.at(name);
}

void VulkanManager::RegisterFrameDependentDescriptorSet(const std::string& name, std::shared_ptr<Vk::DescriptorSet> set, uint32_t frameIndex)
{
	if (frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return;

	frameDependentDescriptorSets[name][frameIndex] = set;
}

std::shared_ptr<Vk::DescriptorSet> VulkanManager::GetFrameDependentDescriptorSet(const std::string& name, uint32_t frameIndex) const
{
	if (frameDependentDescriptorSets.find(name) == frameDependentDescriptorSets.end() || frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return nullptr;

	return frameDependentDescriptorSets.at(name)[frameIndex];
}

void VulkanManager::RegisterDescriptorSet(const std::string& name, std::shared_ptr<Vk::DescriptorSet> set)
{
	descriptorSets[name] = set;
}

std::shared_ptr<Vk::DescriptorSet> VulkanManager::GetDescriptorSet(const std::string& name) const
{
	if (descriptorSets.find(name) == descriptorSets.end())
		return nullptr;

	return descriptorSets.at(name);
}

void VulkanManager::RegisterPushDescriptorSet(const std::string& name, std::shared_ptr<Vk::PushDescriptorSet> set)
{
	pushDescriptorSets[name] = set;
}

std::shared_ptr<Vk::PushDescriptorSet> VulkanManager::GetPushDescriptorSet(const std::string& name) const
{
	if (pushDescriptorSets.find(name) == pushDescriptorSets.end())
		return nullptr;

	return pushDescriptorSets.at(name);
}

void VulkanManager::RegisterFrameDependentFence(const std::string& name, std::shared_ptr<Vk::Fence> fence, uint32_t frameIndex)
{
	if (frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return;

	frameDependentFences[name][frameIndex] = fence;
}

std::shared_ptr<Vk::Fence> VulkanManager::GetFrameDependentFence(const std::string& name, uint32_t frameIndex)
{
	if (frameDependentFences.find(name) == frameDependentFences.end() || frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return nullptr;

	return frameDependentFences.at(name)[frameIndex];
}

void VulkanManager::RegisterFrameDependentSemaphore(const std::string& name, std::shared_ptr<Vk::Semaphore> semaphore, uint32_t frameIndex)
{
	if (frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return;

	frameDependentSemaphores[name][frameIndex] = semaphore;
}

std::shared_ptr<Vk::Semaphore> VulkanManager::GetFrameDependentSemaphore(const std::string& name, uint32_t frameIndex)
{
	if (frameDependentSemaphores.find(name) == frameDependentSemaphores.end() || frameIndex >= GlobalConfig::FrameConfig::maxFramesInFlights)
		return nullptr;

	return frameDependentSemaphores.at(name)[frameIndex];
}

void VulkanManager::InitSamplers()
{
	{
		Vk::ImageSamplerConfig config{};
		config.magFilter = VK_FILTER_LINEAR;
		config.minFilter = VK_FILTER_LINEAR;
		config.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		config.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		config.mipMapFilter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		config.anisotropyEnable = false;
		config.maxLod = Vk::MAX_SAMPLER_MIMAP_LEVEL;

		RegisterSampler("Linear", config);
	}

	{
		Vk::ImageSamplerConfig config{};
		config.magFilter = VK_FILTER_NEAREST;
		config.minFilter = VK_FILTER_NEAREST;
		config.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		config.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		config.mipMapFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		config.anisotropyEnable = false;
		config.maxLod = Vk::MAX_SAMPLER_MIMAP_LEVEL;

		RegisterSampler("Nearest", config);
	}

	{
		Vk::ImageSamplerConfig config{};
		config.magFilter = VK_FILTER_LINEAR;
		config.minFilter = VK_FILTER_LINEAR;
		config.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		config.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		config.mipMapFilter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		config.anisotropyEnable = true;
		config.maxLod = Vk::MAX_SAMPLER_MIMAP_LEVEL;

		RegisterSampler("LinearAniso", config);
	}

	{
		Vk::ImageSamplerConfig config{};
		config.magFilter = VK_FILTER_NEAREST;
		config.minFilter = VK_FILTER_NEAREST;
		config.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		config.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		config.mipMapFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		config.anisotropyEnable = true;
		config.maxLod = Vk::MAX_SAMPLER_MIMAP_LEVEL;

		RegisterSampler("NearestAniso", config);
	}

	{
		Vk::ImageSamplerConfig config{};
		config.magFilter = VK_FILTER_LINEAR;
		config.minFilter = VK_FILTER_LINEAR;
		config.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		config.mipMapFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		config.minLod = 0;
		config.maxLod = 16.f;
		config.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
		config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK; //Irrelevant
		
		RegisterSampler("MaxReduction", config);
	}

	{
		/*
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.compareEnable = VK_TRUE;  // <-- FONTOS!
		samplerInfo.compareOp = VK_COMPARE_OP_LESS; // vagy más opció (LEQUAL, GREATER, stb.)
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		*/
	}
}

void VulkanManager::InitRenderPasses()
{
	/*
		Vk::RenderPassBuilder renderPassBuilder;
		renderPassBuilder.RegisterSubpass("mainSubpass", 0);
		renderPassBuilder.AttachImageDescription("main", 0, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		renderPassBuilder.AttachImageReferenceToSubpass("mainSubpass", "main", VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		renderPassBuilder.AttachDepthDescription(1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		renderPassBuilder.AttachDepthReferenceToSubpass("mainSubpass", VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		renderPassBuilder.AttachSubpassDependency("mainSubpass", Vk::DependencyStage::SRC, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_NONE);
		renderPassBuilder.AttachSubpassDependency("mainSubpass", Vk::DependencyStage::DST, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		renderPass = renderPassBuilder.BuildRenderPass();
	*/
}

void VulkanManager::InitFrameBuffers()
{
	Vk::ImageSpecification mainImageSpec;
	mainImageSpec.type = VK_IMAGE_TYPE_2D;
	mainImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	mainImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	mainImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	mainImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	mainImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	mainImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::ImageSpecification positionImageSpec;
	positionImageSpec.type = VK_IMAGE_TYPE_2D;
	positionImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	positionImageSpec.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	positionImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	positionImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	positionImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	positionImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::ImageSpecification colorImageSpec;
	colorImageSpec.type = VK_IMAGE_TYPE_2D;
	colorImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	colorImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	colorImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	colorImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::ImageSpecification normalImageSpec;
	normalImageSpec.type = VK_IMAGE_TYPE_2D;
	normalImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	normalImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	normalImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	normalImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	normalImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	normalImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::ImageSpecification entityImageSpec;
	entityImageSpec.type = VK_IMAGE_TYPE_2D;
	entityImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	entityImageSpec.format = VK_FORMAT_R32_UINT;
	entityImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	entityImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	entityImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	entityImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::ImageSpecification depthPyramidImageSpec;
	depthPyramidImageSpec.type = VK_IMAGE_TYPE_2D;
	depthPyramidImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthPyramidImageSpec.format = VK_FORMAT_R32_SFLOAT;
	depthPyramidImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthPyramidImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthPyramidImageSpec.aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	depthPyramidImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	depthPyramidImageSpec.calcualteMipLevelAutomaticly = true;

	Vk::ImageSpecification depthImageSpec;
	depthImageSpec.type = VK_IMAGE_TYPE_2D;
	depthImageSpec.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
	depthImageSpec.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthImageSpec.aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImageSpec.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	Vk::FrameBufferBuilder frameBufferBuilder;
	frameBufferBuilder.SetSize(4, 4)
		.AddImageSpecification("Main", 0, mainImageSpec)
		.AddImageSpecification("Position", 1, positionImageSpec)
		.AddImageSpecification("Color", 2, colorImageSpec)
		.AddImageSpecification("Normal", 3, normalImageSpec)
		.AddImageSpecification("Entity", 4, entityImageSpec)
		.AddImageSpecification("DepthPyramid", 5, depthPyramidImageSpec)
		.AddDepthSpecification(6, depthImageSpec);

	for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
		RegisterFrameDependentFrameBuffer("Main", frameBufferBuilder.BuildDynamic(), i);
}

void VulkanManager::InitDescriptors()
{
	{
		Vk::DescriptorPoolBuilder poolBuilder;
		poolBuilder
			.SetMaxSets(GlobalConfig::FrameConfig::maxFramesInFlights)
			.SetPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 10)
			.SetPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
			.SetPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10);
	
		RegisterDescriptorPool("Main", poolBuilder.BuildDescriptorPool());

		for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
			InitMainFramebufferDescriptorSet(i);
	}

	{
		constexpr uint32_t MAX_SAMPLERS = 16;
		constexpr uint32_t MAX_IMAGES = 2048;

		Vk::DescriptorPoolBuilder poolBuilder;
		poolBuilder
			.SetMaxSets(1)
			.SetPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 16)
			.SetPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2048);
	
		RegisterDescriptorPool("ImagePool", poolBuilder.BuildDescriptorPool());

		Vk::DescriptorSetBuilder setBuilder;
		setBuilder
			.AddDescriptorLayoutImage("Samplers", 0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED, MAX_SAMPLERS)
			.AddDescriptorLayoutImage("Images", 1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, MAX_IMAGES);

		RegisterDescriptorSet("LoadedImages", setBuilder.BuildDescriptorSet(GetDescriptorPool("ImagePool")->Value()));

		GetDescriptorSet("LoadedImages")->UpdateImageArrayElement("Samplers", VK_NULL_HANDLE, GetSampler("Nearest")->Value(), 0);
		GetDescriptorSet("LoadedImages")->UpdateImageArrayElement("Samplers", VK_NULL_HANDLE, GetSampler("Linear")->Value(), 1);
		GetDescriptorSet("LoadedImages")->UpdateImageArrayElement("Samplers", VK_NULL_HANDLE, GetSampler("NearestAniso")->Value(), 2);
		GetDescriptorSet("LoadedImages")->UpdateImageArrayElement("Samplers", VK_NULL_HANDLE, GetSampler("LinearAniso")->Value(), 3);
		GetDescriptorSet("LoadedImages")->UpdateImageArrayElement("Samplers", VK_NULL_HANDLE, GetSampler("MaxReduction")->Value(), 4);
	}

	//Push Descriptor
	{
		Vk::DescriptorSetBuilder setBuilder;
		setBuilder
			.AddDescriptorLayoutImage("SrcImage", 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL)
			.AddDescriptorLayoutImage("DstImage", 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL);
	
		RegisterPushDescriptorSet("HizPushSet", setBuilder.BuildPushDescriptorSet());
	}
}


void VulkanManager::InitMainFramebufferDescriptorSet(uint32_t frameIndex)
{
	Vk::DescriptorSetBuilder setBuilder;
	setBuilder
		.AddDescriptorLayoutImage("FboMain", 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, GetFrameDependentFrameBuffer("Main", frameIndex)->GetImage("Main")->GetImageView(), GetSampler("Nearest")->Value(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.AddDescriptorLayoutImage("FboPosition", 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, GetFrameDependentFrameBuffer("Main", frameIndex)->GetImage("Position")->GetImageView(), GetSampler("Nearest")->Value(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.AddDescriptorLayoutImage("FboColor", 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, GetFrameDependentFrameBuffer("Main", frameIndex)->GetImage("Color")->GetImageView(), GetSampler("Nearest")->Value(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.AddDescriptorLayoutImage("FboNormal", 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, GetFrameDependentFrameBuffer("Main", frameIndex)->GetImage("Normal")->GetImageView(), GetSampler("Nearest")->Value(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		.AddDescriptorLayoutImage("FboEntity", 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, GetFrameDependentFrameBuffer("Main", frameIndex)->GetImage("Entity")->GetImageView(), GetSampler("Nearest")->Value(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	RegisterFrameDependentDescriptorSet("MainFrameBuffer", setBuilder.BuildDescriptorSet(GetDescriptorPool("Main")->Value()), frameIndex);
}


void VulkanManager::InitShaderModuls()
{
	//Basic Shaders
	RegisterShaderModule("BasicVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Basic.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("BasicFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Basic.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Geometry Shaders (Deferred Pre)
	RegisterShaderModule("DeferredPreVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPre.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("DeferredPreFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPre.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Geometry Shaders Indirect (Deferred Pre)
	RegisterShaderModule("DeferredPreIndirectVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPreIndirect.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("DeferredPreIndirectFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPreIndirect.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Deferred Direction Light Shaders
	RegisterShaderModule("DeferredDirectionLightVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredDirectionLight.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("DeferredDirectionLightFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredDirectionLight.frag", VK_SHADER_STAGE_FRAGMENT_BIT));
	
	//Deferred Point Light Shaders
	RegisterShaderModule("DeferredPointLightVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPointLight.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("DeferredPointLightFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredPointLight.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Deferred Spot Light Shaders
	RegisterShaderModule("DeferredSpotLightVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredSpotLight.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("DeferredSpotLightFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/DeferredSpotLight.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Bounding Volume Wireframe Shaders
	RegisterShaderModule("WireframeVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Wireframe.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("WireframeFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Wireframe.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//OcclusionCulling
	RegisterShaderModule("OcclusionCullingVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/OcclusionCulling.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("OcclusionCullingFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/OcclusionCulling.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Billboard
	RegisterShaderModule("BillboardInstancedVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/BillboardInstanced.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("BillboardVert", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Billboard.vert", VK_SHADER_STAGE_VERTEX_BIT));
	RegisterShaderModule("BillboardGeom", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Billboard.geom", VK_SHADER_STAGE_GEOMETRY_BIT));
	RegisterShaderModule("BillboardFrag", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Billboard.frag", VK_SHADER_STAGE_FRAGMENT_BIT));

	//Frustum Culling
	RegisterShaderModule("CullingCameraFrustumComp", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/CullingCameraFrustum.comp", VK_SHADER_STAGE_COMPUTE_BIT));

	//Point Light Culling
	RegisterShaderModule("CullingPointLightComp", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/CullingPointLight.comp", VK_SHADER_STAGE_COMPUTE_BIT));

	//Spot Light Culling
	RegisterShaderModule("CullingSpotLightComp", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/CullingSpotLight.comp", VK_SHADER_STAGE_COMPUTE_BIT));
	
	//Object culling against light shadow aabb
	RegisterShaderModule("CullingLightShadowAabbComp", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/CullingLightShadowAabb.comp", VK_SHADER_STAGE_COMPUTE_BIT));

	//Object culling against light shadow aabb
	RegisterShaderModule("HizComp", std::make_shared<Vk::ShaderModule>("../Engine/Shaders/Hiz.comp", VK_SHADER_STAGE_COMPUTE_BIT));
}

void VulkanManager::InitGraphicsPipelines()
{
	{
		uint32_t pushConsantSize = sizeof(VkDeviceAddress);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["BasicVert"])
			.AddShaderStage(shaderModuls["BasicFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT);

		RegisterGraphicsPipeline("Basic", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(GeometryRendererPushConstants);
			 
		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["DeferredPreVert"])
			.AddShaderStage(shaderModuls["DeferredPreFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R32G32B32A32_SFLOAT, 0)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 1)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 2)
			.SetColorAttachmentFormats(VK_FORMAT_R32_UINT, 3)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetDescriptorSet("LoadedImages")->Layout());

		RegisterGraphicsPipeline("DeferredPre", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(GeometryRendererIndirectPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["DeferredPreIndirectVert"])
			.AddShaderStage(shaderModuls["DeferredPreIndirectFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R32G32B32A32_SFLOAT, 0)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 1)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 2)
			.SetColorAttachmentFormats(VK_FORMAT_R32_UINT, 3)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetDescriptorSet("LoadedImages")->Layout());

		RegisterGraphicsPipeline("DeferredPreIndirect", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(DeferredDirectionLightPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["DeferredDirectionLightVert"])
			.AddShaderStage(shaderModuls["DeferredDirectionLightFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetFrameDependentDescriptorSet("MainFrameBuffer", 0)->Layout());

		RegisterGraphicsPipeline("DeferredDirectionLight", pipelineBuilder.BuildDynamic());		
	}

	{
		uint32_t pushConsantSize = sizeof(DeferredPointLightPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["DeferredPointLightVert"])
			.AddShaderStage(shaderModuls["DeferredPointLightFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_FALSE, VK_COMPARE_OP_GREATER_OR_EQUAL)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetFrameDependentDescriptorSet("MainFrameBuffer", 0)->Layout());

		RegisterGraphicsPipeline("DeferredPointLight", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(DeferredSpotLightPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["DeferredSpotLightVert"])
			.AddShaderStage(shaderModuls["DeferredSpotLightFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_FALSE, VK_COMPARE_OP_GREATER_OR_EQUAL)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetFrameDependentDescriptorSet("MainFrameBuffer", 0)->Layout());

		RegisterGraphicsPipeline("DeferredSpotLight", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(BoundingVolumeRendererPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["WireframeVert"])
			.AddShaderStage(shaderModuls["WireframeFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_LINE, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		RegisterGraphicsPipeline("Wireframe", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(OcclusionCullingPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["OcclusionCullingVert"])
			.AddShaderStage(shaderModuls["OcclusionCullingFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_TRUE, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

		RegisterGraphicsPipeline("OcclusionCulling", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(BillboardPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["BillboardVert"])
			.AddShaderStage(shaderModuls["BillboardGeom"])
			.AddShaderStage(shaderModuls["BillboardFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetColorAttachmentFormats(VK_FORMAT_R32_UINT, 1)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetDescriptorSet("LoadedImages")->Layout());

		RegisterGraphicsPipeline("Billboard", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(BillboardPushConstants);

		Vk::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.ResetToDefault()
			.AddShaderStage(shaderModuls["BillboardInstancedVert"])
			.AddShaderStage(shaderModuls["BillboardGeom"])
			.AddShaderStage(shaderModuls["BillboardFrag"])
			.AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
			.AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
			.SetVertexInput({}, {})
			.SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
			.SetRasterization(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
			.SetMultisampling(VK_SAMPLE_COUNT_1_BIT)
			.SetDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS)
			.SetColorBlend(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.AddColorBlendAttachment(VK_FALSE)
			.SetColorAttachmentFormats(VK_FORMAT_R16G16B16A16_SFLOAT, 0)
			.SetColorAttachmentFormats(VK_FORMAT_R32_UINT, 1)
			.SetDepthAttachmentFormat(VK_FORMAT_D32_SFLOAT)
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddDescriptorSetLayout(GetDescriptorSet("LoadedImages")->Layout());

		RegisterGraphicsPipeline("BillboardInstanced", pipelineBuilder.BuildDynamic());
	}

	
}

void VulkanManager::InitComputePipelines()
{
	{
		uint32_t pushConsantSize = sizeof(CullingCameraFrustumPushConstants);

		Vk::ComputePipelineBuilder pipelineBuilder;
		pipelineBuilder
			.AddShaderStage(shaderModuls["CullingCameraFrustumComp"])
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_COMPUTE_BIT);

		RegisterComputePipeline("CullingCameraFrustum", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(CullingPointLightPushConstants);

		Vk::ComputePipelineBuilder pipelineBuilder;
		pipelineBuilder
			.AddShaderStage(shaderModuls["CullingPointLightComp"])
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_COMPUTE_BIT);

		RegisterComputePipeline("CullingPointLight", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(CullingSpotLightPushConstants);

		Vk::ComputePipelineBuilder pipelineBuilder;
		pipelineBuilder
			.AddShaderStage(shaderModuls["CullingSpotLightComp"])
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_COMPUTE_BIT);

		RegisterComputePipeline("CullingSpotLight", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(CullingLightShadowAabbPushConstants);

		Vk::ComputePipelineBuilder pipelineBuilder;
		pipelineBuilder
			.AddShaderStage(shaderModuls["CullingLightShadowAabbComp"])
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_COMPUTE_BIT);

		RegisterComputePipeline("CullingLightShadowAabb", pipelineBuilder.BuildDynamic());
	}

	{
		uint32_t pushConsantSize = sizeof(HizPushConstants);

		Vk::ComputePipelineBuilder pipelineBuilder;
		pipelineBuilder
			.AddShaderStage(shaderModuls["HizComp"])
			.AddPushConstant(0, pushConsantSize, VK_SHADER_STAGE_COMPUTE_BIT)
			.AddDescriptorSetLayout(GetPushDescriptorSet("HizPushSet")->Layout());

		RegisterComputePipeline("Hiz", pipelineBuilder.BuildDynamic());
	}
}

void VulkanManager::InitFences()
{
	for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
		RegisterFrameDependentFence("InFlight", std::make_shared<Vk::Fence>(true), i);
}

void VulkanManager::InitSemaphores()
{
	for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
	{
		RegisterFrameDependentSemaphore("ImageAvailable", std::make_shared<Vk::Semaphore>(), i);
		RegisterFrameDependentSemaphore("RenderFinished", std::make_shared<Vk::Semaphore>(), i);
	}
}