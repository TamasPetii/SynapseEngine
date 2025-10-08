#include "Engine.h"
#include <random>

Engine::Engine()
{
}

Engine::~Engine()
{
	Cleanup();
}

void Engine::Cleanup()
{
	frameTimer.reset();
	renderManager.reset();
	scene.reset();
	resourceManager.reset();
	Vk::VulkanContext::DestroyContext();
}

void Engine::Initialize()
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	vulkanContext->SetRequiredDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	vulkanContext->SetRequiredDeviceExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	vulkanContext->SetRequiredDeviceExtension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
	vulkanContext->SetRequiredDeviceExtension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	vulkanContext->SetRequiredDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
	//vulkanContext->SetRequiredDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
	//vulkanContext->SetRequiredDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_2_EXTENSION_NAME);
	//vulkanContext->SetRequiredDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
	//vulkanContext->SetRequiredDeviceExtension(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
	vulkanContext->Init();

	frameTimer = std::make_shared<Timer>();
	resourceManager = std::make_shared<ResourceManager>();
	renderManager = std::make_shared<RenderManager>(resourceManager);
	scene = std::make_shared<Scene>(resourceManager);
}

void Engine::SetRequiredWindowExtensions(std::span<const char*> extensionNames)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	vulkanContext->SetRequiredInstanceExtensions(extensionNames);
}

void Engine::SetSurfaceCreationFunction(const std::function<void(const Vk::Instance* const, VkSurfaceKHR* surface)>& function)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	vulkanContext->SetSurfaceCreationFunction(function);
}

void Engine::SetWindowExtentFunction(const std::function<std::pair<int, int>()>& function)
{
	auto vulkanContext = Vk::VulkanContext::GetContext();
	vulkanContext->SetWindowExtentFunction(function);
}

void Engine::SetGuiRenderFunction(const std::function<void(VkCommandBuffer, std::shared_ptr<Registry>, std::shared_ptr<ResourceManager>, uint32_t)>& function)
{
	if (renderManager)
		renderManager->SetGuiRenderFunction(function);
}

void Engine::WindowResizeEvent()
{
	renderManager->RecreateSwapChain();
}

void Engine::Update()
{
	static float time = 0;
	static int counter = 0;
	static float resourceUpdateTime = 0;
	static float resourceUpdateTargetTime = 1.0f / 60.0f;

	frameTimer->Update();

	time += frameTimer->GetFrameDeltaTime();
	resourceUpdateTime += frameTimer->GetFrameDeltaTime();
	counter++;

	if (time > 1)
	{
		resourceManager->GetBenchmarkManager()->AverageBenchmarkTimes();
		resourceManager->GetBenchmarkManager()->ResetBenchmarkTimes();

		std::cout << counter << std::endl;

		time = 0;
		counter = 0;
	}

	//Todo GPU BUFFER CHANGE inside fence
	resourceManager->GetBenchmarkManager()->AddToCounter();
	resourceManager->GetModelManager()->Update(frameIndex);
	resourceManager->GetGeometryManager()->Update(frameIndex);
	resourceManager->GetImageManager()->Update();
	resourceManager->GetMaterialManager()->Update(frameIndex);
	resourceManager->GetAnimationManager()->Update(frameIndex);
	resourceManager->GetPointLightBufferManager()->Update(scene->GetRegistry(), frameIndex);
	resourceManager->GetSpotLightBufferManager()->Update(scene->GetRegistry(), frameIndex);

	/*
	if (resourceUpdateTime >= resourceUpdateTargetTime)
	{
		resourceManager->GetModelManager()->Update();
		resourceManager->GetImageManager()->Update();
		resourceManager->GetAnimationManager()->Update();
		resourceManager->GetMaterialManager()->Update();
		resourceUpdateTime -= resourceUpdateTargetTime;

		std::cout << std::format("Resources Update: {}", resourceUpdateTime) << std::endl;
	}
	*/

	scene->Update(frameTimer, frameIndex);
	InputManager::Instance()->UpdatePrevious();
}

void Engine::UpdateGPU()
{
	scene->UpdateGPU(frameIndex);
}

void Engine::Render()
{
	renderManager->Render(scene->GetRegistry(), frameIndex);
}

void Engine::SimulateFrame()
{
	Update();

	auto device = Vk::VulkanContext::GetContext()->GetDevice();
	auto inFlightFence = resourceManager->GetVulkanManager()->GetFrameDependentFence("InFlight", frameIndex);

	vkWaitForFences(device->Value(), 1, &inFlightFence->Value(), VK_TRUE, UINT64_MAX);
	vkResetFences(device->Value(), 1, &inFlightFence->Value());

	/*
	{
		auto pointLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(
			resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler()
			);

		auto pointLightDispatchIndirectBufferHandler = static_cast<VkDispatchIndirectCommand*>(
			resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler()
			);

		auto pointLightCommonDataBufferHandler = static_cast<LightBufferCommonData*>(
			resourceManager->GetPointLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler()
			);

		std::cout << std::format(
			"Point Light: Count: {} | InstanceCount: {} | ShadowCount: {} | ShadowDispatchCount: {}\n",
			pointLightCommonDataBufferHandler[0].count,
			pointLightIndirectDrawBufferHandler[0].instanceCount,
			pointLightCommonDataBufferHandler[0].shadowCount,
			pointLightDispatchIndirectBufferHandler[0].x
		);
	}

	
	{
		auto spotLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(
			resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler()
			);

		auto spotLightDispatchIndirectBufferHandler = static_cast<VkDispatchIndirectCommand*>(
			resourceManager->GetSpotLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler()
			);

		auto spotLightCommonDataBufferHandler = static_cast<LightBufferCommonData*>(
			resourceManager->GetSpotLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler()
			);

		std::cout << std::format(
			"Spot Light: Count: {} | InstanceCount: {} | ShadowCount: {} | ShadowDispatchCount: {}\n",
			spotLightCommonDataBufferHandler[0].count,
			spotLightIndirectDrawBufferHandler[0].instanceCount,
			spotLightCommonDataBufferHandler[0].shadowCount,
			spotLightDispatchIndirectBufferHandler[0].x
		);
	}
	*/



	UpdateGPU();
	Render();

	Finish();

	frameIndex = (frameIndex + 1) % GlobalConfig::FrameConfig::framesInFlight;
}

void Engine::Finish()
{
	scene->Finish();
}