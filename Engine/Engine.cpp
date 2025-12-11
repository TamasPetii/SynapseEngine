#include "Engine.h"
#include <random>
#include "Render/Renderers/DepthHierarchyBuilder.h"
#include <iostream>
#include <format>
#include <numeric>

CullingFrameStats Engine::stats;

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
	vulkanContext->SetRequiredDeviceExtension(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
	vulkanContext->SetRequiredDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
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

	//After scene update: PointLight useshadow changed component pool!
	resourceManager->GetPointLightShadowBufferManager()->Update(scene->GetRegistry(), frameIndex);
	resourceManager->GetSpotLightShadowBufferManager()->Update(scene->GetRegistry(), frameIndex);

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
    //Todo: Update itt kint!

	auto device = Vk::VulkanContext::GetContext()->GetDevice();
	auto inFlightFence = resourceManager->GetVulkanManager()->GetFrameDependentFence("InFlight", frameIndex);

	vkWaitForFences(device->Value(), 1, &inFlightFence->Value(), VK_TRUE, UINT64_MAX);
	vkResetFences(device->Value(), 1, &inFlightFence->Value());

    CollectCullingStats(frameIndex);

	Update();
	UpdateGPU();
	Render();

	Finish();

	frameIndex = (frameIndex + 1) % GlobalConfig::FrameConfig::framesInFlight;
}

void Engine::Finish()
{
	scene->Finish();
}

void Engine::CollectCullingStats(uint32_t frameIndex)
{
    stats = CullingFrameStats{};

    auto modelMgr = resourceManager->GetModelManager();
    auto geoMgr = resourceManager->GetGeometryManager();

    // --- Main Camera ---
    if (auto modelIndBuf = modelMgr->GetIndirectDrawBuffer(frameIndex))
    {
        if (modelIndBuf->buffer)
        {
            auto* cmd = static_cast<VkDrawIndirectCommand*>(modelIndBuf->buffer->GetHandler());
            for (const auto& [name, model] : modelMgr->GetModels())
            {
                stats.mainModelCount += cmd[model->object->GetBufferArrayIndex()].instanceCount;
            }
        }
    }

    if (auto shapeIndBuf = geoMgr->GetIndirectDrawBuffer(frameIndex))
    {
        if (shapeIndBuf->buffer)
        {
            auto* cmd = static_cast<VkDrawIndirectCommand*>(shapeIndBuf->buffer->GetHandler());
            for (const auto& [name, shape] : geoMgr->GetShapes())
            {
                stats.mainShapeCount += cmd[shape->object->GetBufferArrayIndex()].instanceCount;
            }
        }
    }

    // --- Point Lights ---
    auto plBufMgr = resourceManager->GetPointLightBufferManager();
    auto plShadowMgr = resourceManager->GetPointLightShadowBufferManager();

    if (auto commonBuf = plBufMgr->GetCommonDataBuffer(frameIndex))
    {
        if (commonBuf->buffer)
        {
            auto* data = static_cast<LightBufferCommonData*>(commonBuf->buffer->GetHandler());
            stats.pointLightCount = data->count;
            stats.pointShadowCount = data->shadowCount;
			stats.pointObjectCount = data->objectCount;
        }
    }

    if (auto dispatchBuf = plBufMgr->GetShadowDispatchIndirectBuffers(frameIndex))
    {
        if (dispatchBuf->buffer)
        {
            auto* cmd = static_cast<VkDispatchIndirectCommand*>(dispatchBuf->buffer->GetHandler());
            stats.pointDispatchX = cmd->x;
            stats.pointDispatchY = cmd->y;
        }
    }

    for (uint32_t i = 0; i < stats.pointShadowCount; ++i)
    {
        LightShadowStats lightStats;
        lightStats.shadowIndex = i;

        const auto& res = plShadowMgr->GetShadowResources(frameIndex, i);

        if (res.modelIndirectDrawBuffer && res.modelIndirectDrawBuffer->buffer)
        {
            auto* cmds = static_cast<VkDrawIndirectCommand*>(res.modelIndirectDrawBuffer->buffer->GetHandler());

            for (const auto& [name, model] : modelMgr->GetModels())
            {
                uint32_t idx = model->object->GetBufferArrayIndex();
                uint32_t count = cmds[idx].instanceCount;

                if (count > 0 && count != 9999)
                {
                    VisibleInstanceData instanceData;
                    instanceData.name = name;
                    instanceData.count = count;

                    auto instBuffer = plShadowMgr->GetShadowResources(frameIndex, i).modelInstanceIndexBuffers[idx];
                    if (instBuffer && instBuffer->buffer)
                    {
                        uint32_t* indices = static_cast<uint32_t*>(instBuffer->buffer->GetHandler());
                        instanceData.instanceIndices.assign(indices, indices + count);
                    }
                    lightStats.visibleModels.push_back(instanceData);
                }
            }
        }

        if (res.shapeIndirectDrawBuffer && res.shapeIndirectDrawBuffer->buffer)
        {
            auto* cmds = static_cast<VkDrawIndirectCommand*>(res.shapeIndirectDrawBuffer->buffer->GetHandler());

            for (const auto& [name, shape] : geoMgr->GetShapes())
            {
                uint32_t idx = shape->object->GetBufferArrayIndex();
                uint32_t count = cmds[idx].instanceCount;

                if (count > 0 && count != 9999)
                {
                    VisibleInstanceData instanceData;
                    instanceData.name = name;
                    instanceData.count = count;

                    auto instBuffer = plShadowMgr->GetShadowResources(frameIndex, i).shapeInstanceIndexBuffers[idx];
                    if (instBuffer && instBuffer->buffer)
                    {
                        uint32_t* indices = static_cast<uint32_t*>(instBuffer->buffer->GetHandler());
                        instanceData.instanceIndices.assign(indices, indices + count);
                    }
                    lightStats.visibleShapes.push_back(instanceData);
                }
            }
        }
        stats.pointShadowDetails.push_back(lightStats);
    }

    // --- Spot Lights ---
    auto slBufMgr = resourceManager->GetSpotLightBufferManager();
    auto slShadowMgr = resourceManager->GetSpotLightShadowBufferManager();

    if (auto commonBuf = slBufMgr->GetCommonDataBuffer(frameIndex))
    {
        if (commonBuf->buffer)
        {
            auto* data = static_cast<LightBufferCommonData*>(commonBuf->buffer->GetHandler());
            stats.spotLightCount = data->count;
            stats.spotShadowCount = data->shadowCount;
			stats.spotObjectCount = data->objectCount;
        }
    }

    if (auto dispatchBuf = slBufMgr->GetShadowDispatchIndirectBuffers(frameIndex))
    {
        if (dispatchBuf->buffer)
        {
            auto* cmd = static_cast<VkDispatchIndirectCommand*>(dispatchBuf->buffer->GetHandler());
            stats.spotDispatchX = cmd->x;
            stats.spotDispatchY = cmd->y;
        }
    }

    for (uint32_t i = 0; i < stats.spotShadowCount; ++i)
    {
        LightShadowStats lightStats;
        lightStats.shadowIndex = i;

        const auto& res = slShadowMgr->GetShadowResources(frameIndex, i);

        if (res.modelIndirectDrawBuffer && res.modelIndirectDrawBuffer->buffer)
        {
            auto* cmds = static_cast<VkDrawIndirectCommand*>(res.modelIndirectDrawBuffer->buffer->GetHandler());

            for (const auto& [name, model] : modelMgr->GetModels())
            {
                uint32_t idx = model->object->GetBufferArrayIndex();
                uint32_t count = cmds[idx].instanceCount;

                if (count > 0 && count != 9999)
                {
                    VisibleInstanceData instanceData;
                    instanceData.name = name;
                    instanceData.count = count;

                    auto instBuffer = slShadowMgr->GetShadowResources(frameIndex, i).modelInstanceIndexBuffers[idx];
                    if (instBuffer && instBuffer->buffer)
                    {
                        uint32_t* indices = static_cast<uint32_t*>(instBuffer->buffer->GetHandler());
                        instanceData.instanceIndices.assign(indices, indices + count);
                    }
                    lightStats.visibleModels.push_back(instanceData);
                }
            }
        }

        if (res.shapeIndirectDrawBuffer && res.shapeIndirectDrawBuffer->buffer)
        {
            auto* cmds = static_cast<VkDrawIndirectCommand*>(res.shapeIndirectDrawBuffer->buffer->GetHandler());

            for (const auto& [name, shape] : geoMgr->GetShapes())
            {
                uint32_t idx = shape->object->GetBufferArrayIndex();
                uint32_t count = cmds[idx].instanceCount;

                if (count > 0 && count != 9999)
                {
                    VisibleInstanceData instanceData;
                    instanceData.name = name;
                    instanceData.count = count;

                    auto instBuffer = slShadowMgr->GetShadowResources(frameIndex, i).shapeInstanceIndexBuffers[idx];
                    if (instBuffer && instBuffer->buffer)
                    {
                        uint32_t* indices = static_cast<uint32_t*>(instBuffer->buffer->GetHandler());
                        instanceData.instanceIndices.assign(indices, indices + count);
                    }
                    lightStats.visibleShapes.push_back(instanceData);
                }
            }
        }
        stats.spotShadowDetails.push_back(lightStats);
    }
}