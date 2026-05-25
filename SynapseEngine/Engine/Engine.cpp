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
#include "Engine/Scene/NatureScene.h"

#include "Engine/Render/RendererFactory.h"
#include "Engine/Physics/JoltPhysicsEngine.h"

#include "Engine/Profiler/DefaultGpuProfiler.h"
#include "Engine/Profiler/DefaultCpuProfiler.h"

#include "Engine/Serialization/Serializer.h"
#include "Engine/Serialization/Archive/DefaultArchiveRegistry.h"
#include "Engine/Serialization/Archive/Output/Json/NlohmannJsonOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Json/NlohmannJsonInputArchive.h"
#include "Engine/Serialization/Archive/Input/Binary/BinaryInputArchive.h"
#include "Engine/Serialization/Archive/Output/Binary/BinaryOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Xml/TinyXmlInputArchive.h"
#include "Engine/Serialization/Archive/Output/Xml/TinyXmlOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Yaml/YamlCppInputArchive.h"
#include "Engine/Serialization/Archive/Output/Yaml/YamlCppOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Toml/PlusPlusTomlInputArchive.h"
#include "Engine/Serialization/Archive/Output/Toml/PlusPlusTomlOutputArchive.h"

#include <print>
#include <filesystem>

#include "Engine/Serialization/Schema/GlmSchema.h"
#include "Engine/Serialization/Schema/VectorSchema.h"
#include "Engine/Serialization/Schema/TransformComponentSchema.h"

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

		InitFrameContext(1);
		InitLogger();
		InitVulkan(params);
		InitTaskExecutor();
		InitSerializer();
		InitResourceManager();
		InitRenderManager(params);
		InitSceneManager();
		InitPhysicsEngine();
		InitProfilers();

		TestSerializer();
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
		_resourceManager = std::make_unique<ResourceManager>(_frameContext.framesInFlight);
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

			std::string logReport = "";

			if (auto cpuProfiler = ServiceLocator::GetCpuProfiler()) {
				logReport += cpuProfiler->GenerateReport(prevFrame, "CPU") + "\n";
			}

			if (auto gpuProfiler = ServiceLocator::GetGpuProfiler()) {
				logReport += gpuProfiler->GenerateReport(prevFrame, "GPU") + "\n";
			}
			std::println("{}\n", logReport);
			std::println("FPS: {} ({} ms/frame)\n", frameCount, 1000.0f / frameCount);

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
		/*
#ifdef SYN_PERFORMANCE
		_renderManager = std::move(RendererFactory::CreatePerformanceRenderer(_frameContext.framesInFlight));
#else
		_renderManager = std::move(RendererFactory::CreateDeferredRenderer(_frameContext.framesInFlight));
#endif
		*/
		_renderManager = std::move(RendererFactory::CreateDeferredRenderer(_frameContext.framesInFlight));
		_renderManager->SetGuiRenderCallback(params.onRenderGuiCallback);
	}

	void Engine::Shutdown() 
	{
		_physicsEngine->Shutdown();
		_physicsEngine.reset();
		_taskExecutor.reset();
		_inputManager.reset();
		_serializer.reset();
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

	void Engine::InitSerializer()
	{
		auto registry = std::make_unique<DefaultArchiveRegistry>();
		registry->RegisterOutputAuto<NlohmannJsonOutputArchive>(10);
		registry->RegisterInputAuto<NlohmannJsonInputArchive>(10);
		registry->RegisterOutputAuto<BinaryOutputArchive>(10);
		registry->RegisterInputAuto<BinaryInputArchive>(10);
		registry->RegisterOutputAuto<TinyXmlOutputArchive>(10);
		registry->RegisterInputAuto<TinyXmlInputArchive>(10);
		registry->RegisterOutputAuto<YamlCppOutputArchive>(10);
		registry->RegisterInputAuto<YamlCppInputArchive>(10);
		registry->RegisterOutputAuto<PlusPlusTomlOutputArchive>(10);
		registry->RegisterInputAuto<PlusPlusTomlInputArchive>(10);

		auto service = std::make_unique<DefaultSerializationService>(std::move(registry));

		_serializer = std::make_unique<Serializer>(std::move(service));

		ServiceLocator::ProvideSerializer(_serializer.get());
	}

	void Engine::TestSerializer()
	{
		const char* appDataPath = std::getenv("APPDATA");
		std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
		std::filesystem::path saveDir = baseDir / "Synapse" / "Saves";

		if (!std::filesystem::exists(saveDir)) {
			std::filesystem::create_directories(saveDir);
		}

		Info("[Serializer] Starting JSON Transform test...");
		std::filesystem::path jsonPath = saveDir / "transform_test.json";

		TransformComponent myTransform;
		myTransform.translation = glm::vec3(100.5f, 50.0f, -25.2f);
		myTransform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
		myTransform.scale = glm::vec3(2.0f, 2.0f, 2.0f);

		{
			Info("[Serializer] --- STARTING SINGLE JSON TEST ---");
			std::filesystem::path jsonPath = saveDir / "transform_test.json";

			if (_serializer->SaveToFile(jsonPath, myTransform)) {
				Info("  > Saved successfully!");
			}

			TransformComponent loadedJsonTransform;
			if (_serializer->LoadFromFile(jsonPath, loadedJsonTransform)) {
				Info("  > Loaded Pos: {}, {}, {}", loadedJsonTransform.translation.x, loadedJsonTransform.translation.y, loadedJsonTransform.translation.z);
			}
		}

		{
			Info("[Serializer] --- STARTING SINGLE BINARY TEST ---");
			std::filesystem::path binPath = saveDir / "transform_test.bin";

			if (_serializer->SaveToFile(binPath, myTransform)) {
				Info("  > Saved successfully to: {}", binPath.string());
			}

			TransformComponent loadedBinTransform;
			if (_serializer->LoadFromFile(binPath, loadedBinTransform)) {
				Info("  > Loaded Pos: {}, {}, {}", loadedBinTransform.translation.x, loadedBinTransform.translation.y, loadedBinTransform.translation.z);
			}
		}

		{
			Info("[Serializer] --- STARTING SINGLE XML TEST ---");
			std::filesystem::path xmlSinglePath = saveDir / "transform_test.xml";

			if (_serializer->SaveToFile(xmlSinglePath, myTransform)) {
				Info("  > Saved successfully to: {}", xmlSinglePath.string());
			}

			TransformComponent loadedXmlTransform;
			if (_serializer->LoadFromFile(xmlSinglePath, loadedXmlTransform)) {
				Info("  > Loaded Pos: {}, {}, {}", loadedXmlTransform.translation.x, loadedXmlTransform.translation.y, loadedXmlTransform.translation.z);
			}
		}

		{
			Info("[Serializer] --- STARTING SINGLE YAML TEST ---");
			std::filesystem::path yamlSinglePath = saveDir / "transform_test.yaml";

			if (_serializer->SaveToFile(yamlSinglePath, myTransform)) {
				Info("  > Saved successfully to: {}", yamlSinglePath.string());
			}

			TransformComponent loadedYamlTransform;
			if (_serializer->LoadFromFile(yamlSinglePath, loadedYamlTransform)) {
				Info("  > Loaded Pos: {}, {}, {}", loadedYamlTransform.translation.x, loadedYamlTransform.translation.y, loadedYamlTransform.translation.z);
			}
		}

		{
			Info("[Serializer] --- STARTING SINGLE TOML TEST ---");
			std::filesystem::path tomlSinglePath = saveDir / "transform_test.toml";

			if (_serializer->SaveToFile(tomlSinglePath, myTransform)) {
				Info("  > Saved successfully to: {}", tomlSinglePath.string());
			}

			TransformComponent loadedTomlTransform;
			if (_serializer->LoadFromFile(tomlSinglePath, loadedTomlTransform)) {
				Info("  > Loaded Pos: {}, {}, {}", loadedTomlTransform.translation.x, loadedTomlTransform.translation.y, loadedTomlTransform.translation.z);
			}
		}

		Info("[Serializer] Generating 10,000 Transforms for benchmark...");
		const int NUM_ELEMENTS = 10000;
		std::vector<TransformComponent> transformsToSave;
		transformsToSave.reserve(NUM_ELEMENTS);

		for (int i = 0; i < NUM_ELEMENTS; ++i) {
			TransformComponent dummyTransform;
			dummyTransform.translation = glm::vec3(i * 1.5f, 0.0f, 0.0f);
			dummyTransform.rotation = glm::vec3(0.0f);
			dummyTransform.scale = glm::vec3(1.0f);
			transformsToSave.push_back(dummyTransform);
		}

		{
			Info("[Serializer] --- STARTING FAST BINARY (DOD) VECTOR TEST ---");
			std::filesystem::path vecBinPath = saveDir / "transform_vector_test.bin";

			BlitVector<TransformComponent> saveArray{ transformsToSave };
			if (_serializer->SaveToFile(vecBinPath, saveArray)) {
				Info("  > 10,000 Transforms saved FAST in binary!");
			}

			std::vector<TransformComponent> loadedTransforms;
			BlitVector<TransformComponent> loadArray{ loadedTransforms };
			if (_serializer->LoadFromFile(vecBinPath, loadArray)) {
				Info("  > Vector loaded! Element count: {}", loadedTransforms.size());
				if (!loadedTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedTransforms.back().translation.x);
				}
			}
		}

		{
			Info("[Serializer] --- STARTING SLOW BINARY VECTOR TEST ---");
			std::filesystem::path vecSlowBinPath = saveDir / "transform_vector_slow_test.bin";

			if (_serializer->SaveToFile(vecSlowBinPath, transformsToSave)) {
				Info("  > 10,000 Transforms saved SLOW in binary!");
			}

			std::vector<TransformComponent> loadedSlowTransforms;
			if (_serializer->LoadFromFile(vecSlowBinPath, loadedSlowTransforms)) {
				Info("  > Slow Vector loaded! Element count: {}", loadedSlowTransforms.size());
				if (!loadedSlowTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedSlowTransforms.back().translation.x);
				}
			}
		}

		{
			Info("[Serializer] --- STARTING JSON VECTOR TEST ---");
			std::filesystem::path vecJsonPath = saveDir / "transform_vector_test.json";

			if (_serializer->SaveToFile(vecJsonPath, transformsToSave)) {
				Info("  > 10,000 Transforms saved in JSON!");
			}

			std::vector<TransformComponent> loadedJsonTransforms;
			if (_serializer->LoadFromFile(vecJsonPath, loadedJsonTransforms)) {
				Info("  > JSON Vector loaded! Element count: {}", loadedJsonTransforms.size());
				if (!loadedJsonTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedJsonTransforms.back().translation.x);
				}
			}
		}

		{
			Info("[Serializer] --- STARTING XML VECTOR TEST ---");
			std::filesystem::path vecXmlPath = saveDir / "transform_vector_test.xml";

			if (_serializer->SaveToFile(vecXmlPath, transformsToSave)) {
				Info("  > 10,000 Transforms saved in XML!");
			}

			std::vector<TransformComponent> loadedXmlTransforms;
			if (_serializer->LoadFromFile(vecXmlPath, loadedXmlTransforms)) {
				Info("  > XML Vector loaded! Element count: {}", loadedXmlTransforms.size());
				if (!loadedXmlTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedXmlTransforms.back().translation.x);
				}
			}
		}

		{
			Info("[Serializer] --- STARTING YAML VECTOR TEST ---");
			std::filesystem::path vecYamlPath = saveDir / "transform_vector_test.yaml";

			if (_serializer->SaveToFile(vecYamlPath, transformsToSave)) {
				Info("  > 10,000 Transforms saved in YAML!");
			}

			std::vector<TransformComponent> loadedYamlTransforms;
			if (_serializer->LoadFromFile(vecYamlPath, loadedYamlTransforms)) {
				Info("  > YAML Vector loaded! Element count: {}", loadedYamlTransforms.size());
				if (!loadedYamlTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedYamlTransforms.back().translation.x);
				}
			}
		}

		{
			Info("[Serializer] --- STARTING TOML VECTOR TEST ---");
			std::filesystem::path vecTomlPath = saveDir / "transform_vector_test.toml";

			if (_serializer->SaveToFile(vecTomlPath, transformsToSave)) {
				Info("  > 10,000 Transforms saved in TOML!");
			}

			std::vector<TransformComponent> loadedTomlTransforms;
			if (_serializer->LoadFromFile(vecTomlPath, loadedTomlTransforms)) {
				Info("  > TOML Vector loaded! Element count: {}", loadedTomlTransforms.size());
				if (!loadedTomlTransforms.empty()) {
					Info("  > Last element Pos X: {}", loadedTomlTransforms.back().translation.x);
				}
			}
		}

		Info("[Serializer] Benchmark complete!");
	}
}