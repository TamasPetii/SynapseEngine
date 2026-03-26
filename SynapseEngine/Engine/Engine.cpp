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

		ServiceLocator::GetAnimationManager()->Update();
		ServiceLocator::GetModelManager()->Update();
		ServiceLocator::GetMaterialManager()->Update();
		ServiceLocator::GetImageManager()->Update();
		ServiceLocator::GetGpuUploader()->ProcessUploads();

		_sceneManager->Update(_frameContext.deltaTime, _frameContext.currentFrameIndex);
	
		ServiceLocator::GetInputManager()->UpdatePrevious();
	}

	void Engine::Render()
	{
		if (_isMinimized)
			return;

		uint32_t currentFrame = _frameContext.currentFrameIndex;

		_renderManager->WaitForFrame(currentFrame);

		if (_onGuiFlushCallback)
			_onGuiFlushCallback(currentFrame);

		_sceneManager->UpdateGPU(currentFrame);

		_renderManager->RenderFrame(currentFrame, _frameContext.framesInFlight, _sceneManager->GetActiveScene());

		_sceneManager->Finish();

		if(_guiTaskObserver)
			std::vector<TaskProfileData> frameTasks = _guiTaskObserver->ExtractFrameData();

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
		_frameContext.currentFrameIndex = (_frameContext.currentFrameIndex + 1) % _frameContext.framesInFlight;

		static auto lastTime = std::chrono::high_resolution_clock::now();
		static uint32_t frameCount = 0;

		auto currentTime = std::chrono::high_resolution_clock::now();
		frameCount++;

		float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

		if (timeDiff >= 1.0f) {
			std::println("FPS: {} ({} ms/frame)", frameCount, 1000.0f / frameCount);

			frameCount = 0;
			lastTime = currentTime;
		}
	}

	void Engine::InitRenderManager(const EngineInitParams& params)
	{
		_renderManager = std::move(RendererFactory::CreateDeferredRenderer(_frameContext.framesInFlight));
		_renderManager->SetGuiRenderCallback(params.onRenderGuiCallback);
	}

	void Engine::Shutdown() 
	{
		if (EnableLogging && _jsonTaskObserver) {
			const char* appDataPath = std::getenv("APPDATA");

			std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
			std::filesystem::path directory = baseDir / "Synapse" / "Profile";

			if (!std::filesystem::exists(directory)) {
				std::filesystem::create_directories(directory);
			}

			std::string filename = std::format("{}/EngineTaskFlowProfile_{}.json",
				directory.string(),
				Syn::LogUtils::GetCurrentTimeForFileName());

			std::ofstream ofs(filename);
			if (ofs.is_open()) {
				_jsonTaskObserver->dump(ofs);
				ofs.close();
				Info("Taskflow profile successfully saved to: {}", filename);
			}
			else {
				Error("Failed to open profile file for writing: {}", filename);
			}
		}

		_guiTaskObserver.reset();
		_jsonTaskObserver.reset();
		_taskExecutor.reset();
		_inputManager.reset();

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
		//_renderManager->OnResize(width, height);
	}

	void Engine::InitTaskExecutor()
	{
		size_t hardwareThreads = std::thread::hardware_concurrency();
		size_t workerThreads = std::max<size_t>(1, hardwareThreads - 1);

		_taskExecutor = std::make_unique<tf::Executor>(workerThreads);

		if (EnableLogging)
		{
			//_guiTaskObserver = _taskExecutor->make_observer<TaskProfilerObserver>();
			_jsonTaskObserver = _taskExecutor->make_observer<tf::TFProfObserver>();
		}

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
}