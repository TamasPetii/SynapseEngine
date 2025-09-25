#pragma once
#include <string>
#include <array>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Engine/Config.h"
#include "Engine/EngineApi.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/FrameBuffer.h"
#include "Engine/Vulkan/ImageSampler.h"
#include "Engine/Vulkan/ShaderModule.h"
#include "Engine/Vulkan/GraphicsPipeline.h"
#include "Engine/Vulkan/DescriptorPool.h"
#include "Engine/Vulkan/DescriptorSet.h"
#include "Engine/Vulkan/Fence.h"
#include "Engine/Vulkan/Semaphore.h"
#include "Engine/Vulkan/ComputePipeline.h"

class ENGINE_API VulkanManager
{
public:
	VulkanManager();
	~VulkanManager();
	void Initialize();
	void Cleanup();

	//Sampler
	void RegisterSampler(const std::string& name, const Vk::ImageSamplerConfig& config);
	void RegisterSampler(const std::string& name, std::shared_ptr<Vk::ImageSampler> sampler);
	std::shared_ptr<Vk::ImageSampler> GetSampler(const std::string& name) const;

	//RenderPass
	void RegisterRenderPass(const std::string& name, std::shared_ptr<Vk::RenderPass> renderPass);
	std::shared_ptr<Vk::RenderPass> GetRenderPass(const std::string& name) const;

	//ShaderModuls
	void RegisterShaderModule(const std::string& name, std::shared_ptr<Vk::ShaderModule> shaderModule);
	std::shared_ptr<Vk::ShaderModule> GetShaderModule(const std::string& name) const;

	//Graphics Pipelines
	void RegisterGraphicsPipeline(const std::string& name, std::shared_ptr<Vk::GraphicsPipeline> graphicsPipeline);
	std::shared_ptr<Vk::GraphicsPipeline> GetGraphicsPipeline(const std::string& name) const;

	//Compute Pipelines
	void RegisterComputePipeline(const std::string& name, std::shared_ptr<Vk::ComputePipeline> computePipeline);
	std::shared_ptr<Vk::ComputePipeline> GetComputePipeline(const std::string& name) const;

	//FrameBuffers
	void MarkFrameBufferToResize(const std::string& name, uint32_t index, uint32_t width, uint32_t height);
	void ResizeMarkedFrameBuffers(uint32_t frameIndex);
	void RegisterFrameDependentFrameBuffer(const std::string& name, std::shared_ptr<Vk::FrameBuffer> frameBuffer, uint32_t frameIndex);
	std::shared_ptr<Vk::FrameBuffer> GetFrameDependentFrameBuffer(const std::string& name, uint32_t frameIndex) const;

	//Descriptor Pools
	void RegisterDescriptorPool(const std::string& name, std::shared_ptr<Vk::DescriptorPool> pool);
	std::shared_ptr<Vk::DescriptorPool> GetDescriptorPool(const std::string& name) const;

	//Descriptor Sets
	void RegisterFrameDependentDescriptorSet(const std::string& name, std::shared_ptr<Vk::DescriptorSet> set, uint32_t frameIndex);
	std::shared_ptr<Vk::DescriptorSet> GetFrameDependentDescriptorSet(const std::string& name, uint32_t frameIndex) const;

	void RegisterDescriptorSet(const std::string& name, std::shared_ptr<Vk::DescriptorSet> set);
	std::shared_ptr<Vk::DescriptorSet> GetDescriptorSet(const std::string& name) const;

	//Fences
	void RegisterFrameDependentFence(const std::string& name, std::shared_ptr<Vk::Fence> fence, uint32_t frameIndex);
	std::shared_ptr<Vk::Fence> GetFrameDependentFence(const std::string& name, uint32_t frameIndex);

	//Semaphores
	void RegisterFrameDependentSemaphore(const std::string& name, std::shared_ptr<Vk::Semaphore> semaphore, uint32_t frameIndex);
	std::shared_ptr<Vk::Semaphore> GetFrameDependentSemaphore(const std::string& name, uint32_t frameIndex);
private:
	void InitSamplers();
	void InitShaderModuls();
	void InitRenderPasses();
	void InitFrameBuffers();
	void InitDescriptors();
	void InitGraphicsPipelines();
	void InitComputePipelines();
	void InitFences();
	void InitSemaphores();
private:
	void InitMainFramebufferDescriptorSet(uint32_t frameIndex);
private:
	std::unordered_map<std::string, std::shared_ptr<Vk::ImageSampler>> samplers;
	std::unordered_map<std::string, std::shared_ptr<Vk::RenderPass>> renderPasses;
	std::unordered_map<std::string, std::shared_ptr<Vk::ShaderModule>> shaderModuls;
	std::unordered_map<std::string, std::shared_ptr<Vk::DescriptorPool>> descriptorPools;
	std::unordered_map<std::string, std::shared_ptr<Vk::GraphicsPipeline>> graphicsPipelines;
	std::unordered_map<std::string, std::shared_ptr<Vk::ComputePipeline>> computePipelines;
	std::unordered_map<std::string, std::shared_ptr<Vk::DescriptorSet>> descriptorSets;
	std::unordered_map<std::string, std::array<std::shared_ptr<Vk::FrameBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights>> frameDependentBuffers;
	std::unordered_map<std::string, std::array<std::shared_ptr<Vk::FrameBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights>> frameDependentFrameBuffers;
	std::unordered_map<std::string, std::array<std::shared_ptr<Vk::DescriptorSet>, GlobalConfig::FrameConfig::maxFramesInFlights>> frameDependentDescriptorSets;
	std::unordered_map<std::string, std::array<std::shared_ptr<Vk::Fence>, GlobalConfig::FrameConfig::maxFramesInFlights>> frameDependentFences;
	std::unordered_map<std::string, std::array<std::shared_ptr<Vk::Semaphore>, GlobalConfig::FrameConfig::maxFramesInFlights>> frameDependentSemaphores;
	std::set<std::tuple<std::string, uint32_t, uint32_t, uint32_t>> frameBuffersToResize;
};

