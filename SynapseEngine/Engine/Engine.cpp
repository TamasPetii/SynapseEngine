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

#include "Engine/Mesh/Loader/MeshLoaders.h"
#include "Engine/Mesh/Processor/MeshProcessors.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Mesh/Factory/MeshFactory.h"

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
	}

	void Engine::Init(const EngineInitParams& params)
	{
		InitLogger();
		InitVulkan(params);
		InitResourceManager();
		InitStaticMeshBuilder();

		//Rendererekbe kell majd inicializálni!
		auto shaderManager = ServiceLocator::GetShaderManager();
		shaderManager->CreateProgram("TestComputeProgram", { "../Engine/Shaders/Test.comp" });
		shaderManager->CreateProgram("ComplexTestProgram", { "../Engine/Shaders/ComplexTest.vert", "../Engine/Shaders/ComplexTest.frag", "../Engine/Shaders/ComplexTest.geom" });
	
		auto bistro = Syn::MeshFactory::LoadFromFile("C:/Users/User/Desktop/Models/Bistro/BistroExterior.fbx");
		auto sphere = Syn::MeshFactory::CreateSphere();
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

		_vkContext = std::make_unique<Syn::Vk::Context>(vkContextParams);
		ServiceLocator::ProvideVkContext(_vkContext.get());
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
			std::make_unique<MeshProcessorPipeline>()
		);

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpLoader>(), 100);
		_staticMeshBuilder->RegisterProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<ColliderProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());
		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());
	}

	void Engine::Shutdown() {
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