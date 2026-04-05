#include "Engine.h"
#include "Logger/SynLog.h"
#include "ServiceLocator.h"
#include "Vk/Context.h"
#include "Vk/Shader/Shader.h"
#include "Vk/Shader/ShaderProgram.h"
#include "Vk/Buffer/SynVkBuffer.h"
#include "Vk/Rendering/GpuUploader.h"

#include "Engine/Manager/ResourceManager.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Converter/DefaultGpuModelConverter.h"
#include "Engine/Mesh/Converter/DefaultModelCooker.h"
#include "Engine/Mesh/Uploader/DefaultGpuModelUploader.h"

#include "Engine/Mesh/Loader/MeshLoaders.h"
#include "Engine/Mesh/Processor/MeshProcessors.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Mesh/Factory/MeshFactory.h"

#include "Engine/Render/RenderManager.h"
#include "Engine/Render/RenderPipeline.h"

#include "Engine/Manager/InputManager.h"
#include "Engine/Logger/LogUtils.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/TestScene.h"

#include "Engine/Render/RendererFactory.h"
#include "Engine/Physics/JoltPhysicsEngine.h"

#include "Engine/Profiler/DefaultGpuProfiler.h"
#include "Engine/Profiler/DefaultCpuProfiler.h"

#include <print>
#include <filesystem>

namespace Syn
{
	Engine::Engine(const EngineInitParams& params)
	{
		Init(params);
	}

	Engine::~Engine()
	{
		Shutdown();
	}

	void Engine::Update(float deltaTime)
	{
		_frameContext.deltaTime = deltaTime;
		uint32_t currentFrame = _frameContext.currentFrameIndex;

		ServiceLocator::GetCpuProfiler()->BeginFrame(currentFrame);

		ServiceLocator::GetAnimationManager()->Update();
		ServiceLocator::GetModelManager()->Update();
		ServiceLocator::GetMaterialManager()->Update();
		ServiceLocator::GetImageManager()->Update();
		ServiceLocator::GetGpuUploader()->ProcessUploads();

		_sceneManager->Update(_frameContext.deltaTime, currentFrame);
	
		ServiceLocator::GetInputManager()->UpdatePrevious();
	}

	void Engine::Render()
	{
		if (_isMinimized)
			return;

		uint32_t currentFrame = _frameContext.currentFrameIndex;

		_renderManager->WaitForFrame(currentFrame);

		ServiceLocator::GetGpuProfiler()->ResolveFrame(currentFrame);

		if (_onGuiFlushCallback)
			_onGuiFlushCallback(currentFrame);

		_sceneManager->UpdateGPU(currentFrame);

		_renderManager->RenderFrame(currentFrame, _frameContext.framesInFlight, _sceneManager->GetActiveScene());

		_sceneManager->Finish();

		ServiceLocator::GetCpuProfiler()->ResolveFrame(currentFrame);

		AdvanceFrameIndex();
	}

	void Engine::Init(const EngineInitParams& params)
	{
		_onGuiFlushCallback = params.onGuiFlushCallback;

		_inputManager = std::make_unique<InputManager>();
		ServiceLocator::ProvideInputManager(_inputManager.get());

		InitFrameContext(3);
		InitLogger();
		InitVulkan(params);
		InitTaskExecutor();
		InitResourceManager();
		InitRenderManager(params);
		InitSceneManager();
		InitPhysicsEngine();
		InitProfilers();
	}

	void Engine::InitLogger()
	{
		Logger::Get().AddSink(std::make_shared<Syn::ConsoleSink>());
		Logger::Get().AddSink(std::make_shared<Syn::MemorySink>());
		Logger::Get().AddSink(std::make_shared<Syn::FileSink>());
	}

	void Engine::InitVulkan(const EngineInitParams& params)
	{
		Vk::ContextInitParams vkContextParams{
			.enableValidation = EnableValidation,
			.getSurfaceExtensionsCallback = params.getSurfaceExtensionsCallback,
			.createSurfaceCallback = params.createSurfaceCallback,
			.getWindowExtentCallback = [=]() -> VkExtent2D {
				auto [width, height] = params.getWindowExtentCallback();
				return VkExtent2D{ width, height };
			}
		};

		_vkContext = std::make_unique<Vk::Context>(vkContextParams);
		ServiceLocator::ProvideVkContext(_vkContext.get());
		_vkContext->InitSwapChain(vkContextParams);

		_gpuUploader = std::make_unique<Vk::GpuUploader>();
		ServiceLocator::ProvideGpuUploader(_gpuUploader.get());
	}

	void Engine::InitResourceManager()
	{
		_resourceManager = std::make_unique<ResourceManager>();
		ServiceLocator::ProvideResourceManager(_resourceManager.get());
	}

	void Engine::InitFrameContext(uint32_t framesInFlight) {
		_frameContext.framesInFlight = framesInFlight;
		_frameContext.currentFrameIndex = 0;
		ServiceLocator::ProvideFrameContext(&_frameContext);
	}

	void Engine::AdvanceFrameIndex() {
		uint32_t prevFrame = _frameContext.currentFrameIndex;

		_frameContext.currentFrameIndex = (_frameContext.currentFrameIndex + 1) % _frameContext.framesInFlight;

		static auto lastTime = std::chrono::high_resolution_clock::now();
		static uint32_t frameCount = 0;

		auto currentTime = std::chrono::high_resolution_clock::now();
		frameCount++;

		float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

		if (timeDiff >= 1.0f) {

			std::string logReport = std::format("FPS: {} ({} ms/frame)\n\n", frameCount, 1000.0f / frameCount);

			if (auto cpuProfiler = ServiceLocator::GetCpuProfiler()) {
				logReport += cpuProfiler->GenerateReport(prevFrame, "CPU") + "\n";
			}

			if (auto gpuProfiler = ServiceLocator::GetGpuProfiler()) {
				logReport += gpuProfiler->GenerateReport(prevFrame, "GPU") + "\n";
			}

			std::println("{}", logReport);

			frameCount = 0;
			lastTime = currentTime;
		}
	}

	void Engine::InitProfilers()
	{
		float timestampPeriod = _vkContext->GetPhysicalDevice()->GetProperties().limits.timestampPeriod;
		_gpuProfiler = std::make_unique<DefaultGpuProfiler>(_frameContext.framesInFlight, timestampPeriod);
		ServiceLocator::ProvideGpuProfiler(_gpuProfiler.get());

		_cpuProfiler = std::make_unique<DefaultCpuProfiler>(_frameContext.framesInFlight);
		ServiceLocator::ProvideCpuProfiler(_cpuProfiler.get());
	}

	void Engine::InitRenderManager(const EngineInitParams& params)
	{
#ifdef SYN_PERFORMANCE
		_renderManager = std::move(RendererFactory::CreatePerformanceRenderer(_frameContext.framesInFlight));
#else
		_renderManager = std::move(RendererFactory::CreateDeferredRenderer(_frameContext.framesInFlight));
#endif
		_renderManager->SetGuiRenderCallback(params.onRenderGuiCallback);
	}

	void Engine::Shutdown() 
	{
		_physicsEngine->Shutdown();
		_physicsEngine.reset();

		_taskExecutor.reset();
		_inputManager.reset();

		_cpuProfiler.reset();
		_gpuProfiler.reset();
		_sceneManager.reset();
		_renderManager.reset();
		_resourceManager.reset();
		_gpuUploader.reset();
		_vkContext.reset(); //This has to be the last one!
		ServiceLocator::Shutdown();
	}

	void Engine::WindowResizeEvent(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) {
			_isMinimized = true;
			return;
		}

		_isMinimized = false;
		_vkContext->GetSwapChain()->Recreate();

#ifdef SYN_PERFORMANCE
		_renderManager->OnResize(width, height);
#endif
	}

	void Engine::InitTaskExecutor()
	{
		size_t hardwareThreads = std::thread::hardware_concurrency();
		size_t workerThreads = std::max<size_t>(1, hardwareThreads - 1);

		_taskExecutor = std::make_unique<tf::Executor>(workerThreads);

		ServiceLocator::ProvideTaskExecutor(_taskExecutor.get());
	}

	void Engine::OnKey(int key, int scancode, int action, int mods)
	{
		if (!_inputEnabled) return;

		if (action == InputAction::PRESS) {
			_inputManager->SetKeyboardKey(key, true);
		}
		else if (action == InputAction::RELEASE) {
			_inputManager->SetKeyboardKey(key, false);
		}
	}

	void Engine::OnMouseButton(int button, int action, int mods)
	{
		if (!_inputEnabled) return;

		if (action == InputAction::PRESS)
		{
			_inputManager->SetMouseButton(button, true);
		}
		else if (action == InputAction::RELEASE)
		{
			_inputManager->SetMouseButton(button, false);
		}
	}

	void Engine::OnMouseMove(float x, float y)
	{
		if (!_inputEnabled) return;
		_inputManager->SetMousePosition(x, y);
	}

	void Engine::InitSceneManager()
	{
		uint32_t frames = _frameContext.framesInFlight;

		_sceneManager = std::make_unique<SceneManager>();
		ServiceLocator::ProvideSceneManager(_sceneManager.get());

		_sceneManager->RegisterScene("TestLevel", [frames]() {
			return std::make_unique<TestScene>(frames);
		});

		_sceneManager->LoadScene("TestLevel");
	}

	void Engine::InitPhysicsEngine()
	{
		_physicsEngine = std::make_unique<JoltPhysicsEngine>();
		_physicsEngine->Init();
		ServiceLocator::ProvidePhysicsEngine(_physicsEngine.get());
	}
}