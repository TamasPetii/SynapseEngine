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
#include "Engine/Render/Data/RenderScene.h"

#include "Engine/Render/GraphicsPass/TestPass.h"
#include "Engine/Render/GraphicsPass/TestMeshPass.h"
#include "Engine/Render/GraphicsPass/PresentationPass.h"
#include "Engine/Render/RenderPipeline.h"

#include "Engine/Manager/InputManager.h"
#include "Engine/Logger/LogUtils.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/TestScene.h"

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

	void Engine::Update()
	{
		ServiceLocator::GetModelManager()->Update();
		ServiceLocator::GetImageManager()->Update();
		ServiceLocator::GetGpuUploader()->ProcessUploads();

		_sceneManager->Update(_frameContext.deltaTime, _frameContext.currentFrameIndex);
	}

	void Engine::Render()
	{
		if (_isMinimized)
			return;

		uint32_t currentFrame = ServiceLocator::GetFrameContext()->currentFrameIndex;

		_renderManager->WaitForFrame(currentFrame);

		_sceneManager->UpdateGPU(currentFrame);

		RenderScene scene;
		_renderManager->RenderFrame(currentFrame, scene);

		_sceneManager->Finish();

		AdvanceFrameIndex();
	}

	void Engine::Init(const EngineInitParams& params)
	{
		_inputManager = std::make_unique<InputManager>();
		ServiceLocator::ProvideInputManager(_inputManager.get());

		InitFrameContext(3);
		InitLogger();
		InitVulkan(params);
		InitTaskExecutor();
		InitResourceManager();
		InitRenderPipelines();
		InitSceneManager();

		auto shaderManager = ServiceLocator::GetShaderManager();
		shaderManager->CreateProgram("TestComputeProgram", { "../Engine/Shaders/Test.comp" });
		shaderManager->CreateProgram("ComplexTestProgram", { "../Engine/Shaders/ComplexTest.vert", "../Engine/Shaders/ComplexTest.frag", "../Engine/Shaders/ComplexTest.geom" });
	
		auto modelManager = ServiceLocator::GetModelManager();
		modelManager->LoadModelAsync("C:/Users/User/Desktop/Models/Sponza-master/sponza.obj");
		modelManager->LoadModelAsync("C:/Users/User/Desktop/Models/Bistro/BistroExterior.fbx");

		modelManager->LoadModelFromStaticMeshAsync("Sphere", []() {
				return MeshFactory::CreateSphere();
			});

		modelManager->LoadModelFromStaticMeshAsync("Cube", []() {
			return MeshFactory::CreateCube();
			});

		modelManager->LoadModelFromStaticMeshAsync("Quad", []() {
			return MeshFactory::CreateQuad();
			});

		modelManager->LoadModelFromStaticMeshAsync("ScreenQuad", []() {
			return MeshFactory::CreateScreenQuad();
			});

		modelManager->LoadModelFromStaticMeshAsync("Cylinder", []() {
			return MeshFactory::CreateCylinder();
			});

		modelManager->LoadModelFromStaticMeshAsync("Cone", []() {
			return MeshFactory::CreateCone();
			});

		modelManager->LoadModelFromStaticMeshAsync("Capsule", []() {
			return MeshFactory::CreateCapsule();
			});

		modelManager->LoadModelFromStaticMeshAsync("Hemisphere", []() {
			return MeshFactory::CreateHemisphere();
			});

		modelManager->LoadModelFromStaticMeshAsync("Pyramid", []() {
			return MeshFactory::CreatePyramid();
			});

		modelManager->LoadModelFromStaticMeshAsync("Grid", []() {
			return MeshFactory::CreateGrid();
			});

		modelManager->LoadModelFromStaticMeshAsync("Torus", []() {
			return MeshFactory::CreateTorus();
			});
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
		//Todo?? _frameContext.deltaTime

		static auto lastTime = std::chrono::high_resolution_clock::now();
		static uint32_t frameCount = 0;

		auto currentTime = std::chrono::high_resolution_clock::now();
		frameCount++;

		float timeDiff = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

		if (timeDiff >= 1.0f) {
			Info("FPS: {} ({} ms/frame)", frameCount, 1000.0f / frameCount);

			frameCount = 0;
			lastTime = currentTime;
		}
	}

	void Engine::InitRenderPipelines()
	{
		uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
		_renderManager = std::make_unique<RenderManager>(framesInFlight);

		auto pipeline = std::make_unique<RenderPipeline>();
		pipeline->AddPass(std::make_unique<TestMeshPass>());
		pipeline->AddPass(std::make_unique<PresentationPass>());
		pipeline->InitializeAll();

		_renderManager->RegisterPipeline("MainPipeline", std::move(pipeline));
	}

	void Engine::Shutdown() {
		if (EnableLogging && _taskObserver) {
			std::string filename = std::format("Profile/EngineTaskFlowProfile_{}.json", Syn::LogUtils::GetCurrentTimeForFileName());

			std::ofstream ofs(filename);
			_taskObserver->dump(ofs);
			ofs.close();
		}

		_taskObserver.reset();
		_taskExecutor.reset();
		_inputManager.reset();

		_sceneManager.reset();
		_renderManager.reset();
		_resourceManager.reset();
		_vkContext.reset(); //This has to be the last one!
		ServiceLocator::Shutdown();
	}

	void Engine::WindowResizeEvent(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) {
			_isMinimized = true;
		}

		_isMinimized = false;
		_vkContext->GetSwapChain()->Recreate();
	}

	void Engine::InitTaskExecutor()
	{
		size_t hardwareThreads = std::thread::hardware_concurrency();
		size_t workerThreads = std::max<size_t>(1, hardwareThreads - 1);

		_taskExecutor = std::make_unique<tf::Executor>(workerThreads);

		if(EnableLogging)
			_taskObserver = _taskExecutor->make_observer<tf::TFProfObserver>();

		ServiceLocator::ProvideTaskExecutor(_taskExecutor.get());
	}

	void Engine::OnKey(int key, int scancode, int action, int mods)
	{
		if (action == InputAction::PRESS) {
			_inputManager->SetKeyboardKey(key, true);
		}
		else if (action == InputAction::RELEASE) {
			_inputManager->SetKeyboardKey(key, false);
		}
	}

	void Engine::OnMouseButton(int button, int action, int mods)
	{
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
		_inputManager->SetMousePosition(x, y);
	}

	void Engine::InitSceneManager()
	{
		uint32_t frames = _frameContext.framesInFlight;

		_sceneManager = std::make_unique<SceneManager>();
		ServiceLocator::ProvideSceneManager(_sceneManager.get());

		_sceneManager->RegisterScene("TestLevel", [frames]() {
			return std::make_shared<TestScene>(frames);
		});

		_sceneManager->LoadScene("TestLevel");
	}
}