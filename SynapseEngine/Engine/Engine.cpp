#include "Engine.h"
#include "Logger/SynLog.h"
#include "ServiceLocator.h"
#include "Vk/Context.h"
#include "Vk/Shader/Shader.h"
#include "Vk/Shader/ShaderProgram.h"
#include "Vk/Buffer/SynVkBuffer.h"

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
#include "Engine/Render/GraphicsPass/PresentationPass.h"
#include "Engine/Render/RenderPipeline.h"

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
	}

	void Engine::Render()
	{
		if (_isMinimized)
			return;

		uint32_t currentFrame = ServiceLocator::GetFrameContext()->currentFrameIndex;

		_renderManager->WaitForFrame(currentFrame);

		//UpdateGPU();
		//RenderScene scene = _sceneManager->GetScene();
		RenderScene scene;
		_renderManager->RenderFrame(currentFrame, scene);

		AdvanceFrameIndex();
	}

	void Engine::Init(const EngineInitParams& params)
	{
		InitFrameContext(3);
		InitLogger();
		InitVulkan(params);
		InitResourceManager();
		InitStaticMeshBuilder();
		InitRenderPipelines();

		//Rendererekbe kell majd inicializálni!
		auto shaderManager = ServiceLocator::GetShaderManager();
		shaderManager->CreateProgram("TestComputeProgram", { "../Engine/Shaders/Test.comp" });
		shaderManager->CreateProgram("ComplexTestProgram", { "../Engine/Shaders/ComplexTest.vert", "../Engine/Shaders/ComplexTest.frag", "../Engine/Shaders/ComplexTest.geom" });
	
		//auto sponza = Syn::MeshFactory::LoadFromFile("C:/Users/User/Desktop/Models/Sponza-master/sponza.obj");

		auto sphere = Syn::MeshFactory::CreateSphere();

		/*
		auto bistro = Syn::MeshFactory::LoadFromFile("C:/Users/User/Desktop/Models/Bistro/BistroExterior.fbx");
		auto cube = Syn::MeshFactory::CreateCube();
		auto quad = Syn::MeshFactory::CreateQuad();
		auto screenQuad = Syn::MeshFactory::CreateScreenQuad();
		auto cylinder = Syn::MeshFactory::CreateCylinder();
		auto cone = Syn::MeshFactory::CreateCone();
		auto capsule = Syn::MeshFactory::CreateCapsule();
		auto hemisphere = Syn::MeshFactory::CreateHemisphere();
		auto pyramid = Syn::MeshFactory::CreatePyramid();
		auto grid = Syn::MeshFactory::CreateGrid();
		auto torus = Syn::MeshFactory::CreateTorus();
		*/
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
		
		//Swapchain images relie on service locator!
		_vkContext->InitSwapChain(vkContextParams);
	}

	void Engine::InitResourceManager()
	{
		_resourceManager = std::make_unique<ResourceManager>();
		ServiceLocator::ProvideResourceManager(_resourceManager.get());
	}

	void Engine::InitStaticMeshBuilder()
	{
		_staticMeshBuilder = std::make_unique<StaticMeshBuilder>(
			std::make_unique<MeshLoaderRegistry>(),
			std::make_unique<MeshProcessorPipeline>(),
			std::make_unique<DefaultGpuModelConverter>(),
			std::make_unique<DefaultModelCooker>()
		);

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpLoader>(), 100);
		_staticMeshBuilder->RegisterProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<ColliderProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());

		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());
	}

	void Engine::InitFrameContext(uint32_t framesInFlight) {
		_frameContext.framesInFlight = framesInFlight;
		_frameContext.currentFrameIndex = 0;
		ServiceLocator::ProvideFrameContext(&_frameContext);
	}

	void Engine::AdvanceFrameIndex() {
		_frameContext.currentFrameIndex = (_frameContext.currentFrameIndex + 1) % _frameContext.framesInFlight;
		//Todo?? _frameContext.deltaTime
	}

	void Engine::InitRenderPipelines()
	{
		uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
		_renderManager = std::make_unique<RenderManager>(framesInFlight);

		auto pipeline = std::make_unique<RenderPipeline>();
		pipeline->AddPass(std::make_unique<TestPass>());
		pipeline->AddPass(std::make_unique<PresentationPass>());
		pipeline->InitializeAll();

		_renderManager->RegisterPipeline("MainPipeline", std::move(pipeline));
	}

	void Engine::Shutdown() {
		_renderManager.reset();
		_staticMeshBuilder.reset();
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
}