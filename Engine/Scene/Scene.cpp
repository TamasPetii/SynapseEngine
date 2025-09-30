#include "Scene.h"
#include <random>
#include <future>
#include <thread>
#include "Engine/Animation/Animation.h"
#include "Engine/Components/RenderIndicesComponent.h"

Scene::Scene(std::shared_ptr<ResourceManager> resourceManager) : 
	resourceManager(resourceManager)
{
	Initialize();
}

Scene::~Scene()
{
	Cleanup();
}

void Scene::Cleanup()
{
	registry.reset();
	systems.clear();
}

void Scene::Initialize()
{
	InitializeSystems();
	InitializeRegistry();
}

void Scene::InitializeRegistry()
{
	registry = std::make_shared<Registry>();

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<float> dist(0, 1);
	std::uniform_int_distribution<uint32_t> distAnimation(0, 3);
	std::uniform_int_distribution<uint32_t> distShape(0, 4);

	{ //Camera
		auto entity = registry->CreateEntity();
		registry->AddComponents<TransformComponent, CameraComponent>(entity);
		registry->GetComponent<CameraComponent>(entity).isMain = true;
	}

	//Point Lights
	{
		auto lightParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(lightParent);

		auto directionLightParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(directionLightParent);

		auto pointLightParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(pointLightParent);

		auto spotLightParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(spotLightParent);

		registry->SetParent(directionLightParent, lightParent);
		registry->SetParent(pointLightParent, lightParent);
		registry->SetParent(spotLightParent, lightParent);

		//Direction Lights
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, DirectionLightComponent>(entity);
			registry->GetComponent<DirectionLightComponent>(entity).strength = 10.f;
			registry->SetParent(entity, directionLightParent);		
		}

		//Point Lights
		{
			for (int i = 0; i < 10; ++i)
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, PointLightComponent>(entity);
				auto [transformComponent, pointLightComponent] = registry->GetComponents<TransformComponent, PointLightComponent>(entity);

				transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 25.f;
				transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;
				transformComponent.scale = glm::vec3(dist(rng)) * 8.f;

				pointLightComponent.color = glm::vec3(dist(rng), dist(rng), dist(rng));
				pointLightComponent.strength = 5.f;

				registry->SetParent(entity, pointLightParent);
			}
		}

		//Spot Lights
		{
			for (int i = 0; i < 10; ++i)
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, SpotLightComponent>(entity);
				auto [transformComponent, spotLightComponent] = registry->GetComponents<TransformComponent, SpotLightComponent>(entity);
				
				transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 25.f;
				transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;
				transformComponent.scale = glm::vec3(1.f, 5.f, 10.f) + glm::vec3(dist(rng), dist(rng), dist(rng)) * glm::vec3(25.f, 10.f, 25.f);
				
				spotLightComponent.color = glm::vec3(dist(rng), dist(rng), dist(rng));
				spotLightComponent.strength = 5.f;

				registry->SetParent(entity, spotLightParent);
			}
		}
		
	}

	//Shapes
	{
		auto shapeParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(shapeParent);

		std::vector<std::string> shapeNames = {
			"Cube",
			"Sphere",
			"Cone",
			"Cylinder",
			"Torus"
		};

		for (uint32_t i = 0; i < 2500; ++i)
		{
			std::string materialName = "Shape" + std::to_string(i++);
			auto [material, wasLoaded] = resourceManager->GetMaterialManager()->RegisterMaterial(materialName);
			material->color = glm::vec4(dist(rng), dist(rng), dist(rng), 1.f);
			material->roughness = dist(rng);
			material->metalness = dist(rng);
			material->SetBit<UPDATE_BIT>();

			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, MaterialComponent, ShapeComponent, DefaultColliderComponent>(entity);

			auto [transformComponent, materialComponent, shapeComponent] = registry->GetComponents<TransformComponent, MaterialComponent, ShapeComponent>(entity);
			transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 50.f;
			transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;

			materialComponent.material = material;

			shapeComponent.shape = resourceManager->GetGeometryManager()->GetShape(shapeNames[distShape(rng)]);

			registry->SetParent(entity, shapeParent);
		}

		for (uint32_t x = 0; x < 0; ++x)
		{
			for (uint32_t y = 0; y < 0; ++y)
			{
				std::string materialName = "Shape" + std::to_string(x) + std::to_string(y);
				auto [material, wasLoaded] = resourceManager->GetMaterialManager()->RegisterMaterial(materialName);

				material->albedoTexture = resourceManager->GetImageManager()->LoadImage("C:/Users/User/Desktop/Metal053C_4K-PNG/Metal053C_4K-PNG_Color.png", ImageLoadMode::Sync);
				//material->normalTexture = resourceManager->GetImageManager()->LoadImage("C:/Users/User/Desktop/Metal053C_4K-PNG/Metal053C_4K-PNG_NormalGL.png", ImageLoadMode::Sync);
				material->metalnessTexture = resourceManager->GetImageManager()->LoadImage("C:/Users/User/Desktop/Metal053C_4K-PNG/Metal053C_4K-PNG_Metalness.png", ImageLoadMode::Sync);
				material->roughnessTexture = resourceManager->GetImageManager()->LoadImage("C:/Users/User/Desktop/Metal053C_4K-PNG/Metal053C_4K-PNG_Roughness.png", ImageLoadMode::Sync);
				
				material->roughness = x / 10.f;
				material->metalness = y / 10.f;
				material->SetBit<UPDATE_BIT>();

				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, MaterialComponent, ShapeComponent, DefaultColliderComponent>(entity);

				auto [transformComponent, materialComponent, shapeComponent] = registry->GetComponents<TransformComponent, MaterialComponent, ShapeComponent>(entity);
				transformComponent.translation = glm::vec3(y, x, 0) * 2.f;

				materialComponent.material = material;

				shapeComponent.shape = resourceManager->GetGeometryManager()->GetShape("Sphere");

				registry->SetParent(entity, shapeParent);
			}
		}
	}

	//Models
	{
		auto modelParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(modelParent);

		for(uint32_t i = 0; i < 1; i++)
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Models/Pikachu/model.obj");
			transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 50.f;
			transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;
			registry->SetParent(entity, modelParent);
		}

		for (uint32_t i = 0; i < 1; i++)
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Models/Otter/RiverOtter.obj");
			transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 50.f;
			transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;
			transformComponent.scale = glm::vec3(0.5);
			registry->SetParent(entity, modelParent);
		}

		/*
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Bistro_v5_2/BistroInterior.fbx");
			modelComponent.hasDirectxNormals = true;

			registry->SetParent(entity, modelParent);
		}

		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Bistro_v5_2/BistroExterior.fbx");
			modelComponent.hasDirectxNormals = true;

			registry->SetParent(entity, modelParent);
		}
		*/

		/*
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			transformComponent.scale = glm::vec3(0.01);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/main1_sponza/NewSponza_Main_Yup_003.fbx");
		}
		*/

		/*
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			transformComponent.scale = glm::vec3(0.01);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/VulkanEngine/Assets/Sponza/sponza.obj");
		}
		*/

		/*
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.hasDirectxNormals = true;
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/EmeraldSquare_v4_1/EmeraldSquare_Day.fbx");
		}
		*/

		/*
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			transformComponent.scale = glm::vec3(0.01);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/main1_sponza/NewSponza_Main_Yup_003.fbx");
		}
		*/
	}

	//Animations
	{
		std::vector<std::string> animationPaths = {
			"C:/Users/User/Desktop/Animations/Worker_Standing.dae",
			"C:/Users/User/Desktop/Animations/Worker_Walking.dae",
			"C:/Users/User/Desktop/Animations/Worker_Running.dae",
			"C:/Users/User/Desktop/Animations/Worker_Dancing.dae"
		};

		auto animationParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(animationParent);

		for (uint32_t i = 0; i < 0; ++i)
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
			animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation(animationPaths[distAnimation(rng)]);
			transformComponent.translation = (glm::vec3(dist(rng), dist(rng), dist(rng)) * 2.f - 1.f) * 50.f;
			transformComponent.rotation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 180.f;

			registry->SetParent(entity, animationParent);
		}
	}
}

void Scene::Update(std::shared_ptr<Timer> frameTimer, uint32_t frameIndex)
{
	/*
	//Update Registry Transforms
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<float> dist(0, 1);

	if (auto transformPool = registry->GetPool<TransformComponent>())
	{
		std::for_each(std::execution::par,
			transformPool->GetDenseIndices().begin(),
			transformPool->GetDenseIndices().end(),
			[&](Entity entity) -> void
			{
				auto transformComponent = transformPool->GetData(entity);
				transformComponent->scale = glm::vec3(glm::sin(frameTimer->GetFrameElapsedTime() * 2 * glm::pi<float>()) + 2);
				transformPool->SetBit<UPDATE_BIT>(entity);
			}
		);
	}
	*/

	//Update Camera Size
	auto viewPortSize = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", frameIndex)->GetSize();
	auto& cameraComponent = registry->GetComponent<CameraComponent>(0);
	if (viewPortSize.width != cameraComponent.width || viewPortSize.height != cameraComponent.height)
	{
		cameraComponent.width = viewPortSize.width;
		cameraComponent.height = viewPortSize.height;
		registry->GetPool<CameraComponent>()->GetBitset(0).set(UPDATE_BIT, true);
	}

	UpdateSystems(frameIndex, frameTimer->GetFrameDeltaTime());
	FinishSystems();
}

void Scene::UpdateGPU(uint32_t frameIndex)
{
	UpdateComponentBuffers(frameIndex);
	UpdateSystemsGPU(frameIndex);
}

void Scene::InitializeSystems()
{
	InitSystem<TransformSystem>();
	InitSystem<CameraSystem>();
	InitSystem<MaterialSystem>();
	InitSystem<ShapeSystem>();
	InitSystem<ModelSystem>();
	InitSystem<InstanceSystem>();
	InitSystem<DefaultColliderSystem>();
	InitSystem<FrustumCullingSystem>();
	InitSystem<AnimationSystem>();
	InitSystem<DirectionLightSystem>();
	InitSystem<PointLightSystem>();
	InitSystem<SpotLightSystem>();
}

void Scene::UpdateSystems(uint32_t frameIndex, float deltaTime)
{
	Timer timer{};

	std::unordered_map<std::type_index, std::future<void>> futures;

	//TODO: DEDICATED THREAD FOR EACH SYSTEM

	auto LaunchSystemUpdateAsync = [&]<typename T>() -> void {
		futures[Unique::typeID<T>()] = std::async(std::launch::async, 
			[&]() -> void {
				UpdateSystem<T>(frameIndex, deltaTime);
			});
	};

	LaunchSystemUpdateAsync.template operator() < TransformSystem > ();
	LaunchSystemUpdateAsync.template operator() < CameraSystem > ();
	LaunchSystemUpdateAsync.template operator() < MaterialSystem > ();
	LaunchSystemUpdateAsync.template operator() < ShapeSystem > ();
	LaunchSystemUpdateAsync.template operator() < AnimationSystem > ();

	//Maybe problematic in long term -> Need a mechanism to handle model indices
	futures[Unique::typeID<AnimationSystem>()].get();
	LaunchSystemUpdateAsync.template operator() < ModelSystem > ();

	//DefaultColliderSystem uses these systems output as input
	futures[Unique::typeID<TransformSystem>()].get();
	LaunchSystemUpdateAsync.template operator() < DirectionLightSystem > ();
	LaunchSystemUpdateAsync.template operator() < PointLightSystem > ();
	LaunchSystemUpdateAsync.template operator() < SpotLightSystem > ();

	futures[Unique::typeID<ShapeSystem>()].get();
	futures[Unique::typeID<ModelSystem>()].get();
	LaunchSystemUpdateAsync.template operator() < DefaultColliderSystem > ();

	//FrustumCullingSystem uses these systems output as input
	futures[Unique::typeID<CameraSystem>()].get();
	futures[Unique::typeID<DefaultColliderSystem>()].get();
	LaunchSystemUpdateAsync.template operator() < FrustumCullingSystem > ();

	//InstanceSystem uses these systems output as input
	futures[Unique::typeID<FrustumCullingSystem>()].get();
	LaunchSystemUpdateAsync.template operator() < InstanceSystem > ();

	for (auto& [_, future] : futures) {
		if(future.valid())
			future.get();
	}

	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<System>(timer.GetElapsedTime());
}

void Scene::FinishSystems()
{
	Timer timer{};

	std::unordered_map<std::type_index, std::future<void>> futures;

	auto LaunchSystemFinishAsync = [&]<typename T>() -> void {
		futures[Unique::typeID<T>()] = std::async(std::launch::async,
			[&]() -> void {
				FinishSystem<T>();
			});
	};

	LaunchSystemFinishAsync.template operator() < TransformSystem > ();
	LaunchSystemFinishAsync.template operator() < CameraSystem > ();
	LaunchSystemFinishAsync.template operator() < MaterialSystem > ();
	LaunchSystemFinishAsync.template operator() < ShapeSystem > ();
	LaunchSystemFinishAsync.template operator() < ModelSystem > ();
	LaunchSystemFinishAsync.template operator() < InstanceSystem > ();
	LaunchSystemFinishAsync.template operator() < DefaultColliderSystem > ();
	LaunchSystemFinishAsync.template operator() < AnimationSystem > ();
	LaunchSystemFinishAsync.template operator() < DirectionLightSystem > ();
	LaunchSystemFinishAsync.template operator() < PointLightSystem > ();
	LaunchSystemFinishAsync.template operator() < SpotLightSystem > ();

	for (auto& [_, future] : futures) {
		if (future.valid())
			future.get();
	}

	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<System>(timer.GetElapsedTime());
}

void Scene::UpdateSystemsGPU(uint32_t frameIndex)
{
	Timer timer{};

	std::unordered_map<std::type_index, std::future<void>> futures;

	auto LaunchSystemUpdateGpuAsync = [&]<typename T>() -> void {
		futures[Unique::typeID<T>()] = std::async(std::launch::async,
			[&]() -> void {
				UpdateGpuSystem<T>(frameIndex);
			});
	};

	LaunchSystemUpdateGpuAsync.template operator() < TransformSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < CameraSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < MaterialSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < ShapeSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < ModelSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < InstanceSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < DefaultColliderSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < AnimationSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < DirectionLightSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < PointLightSystem > ();
	LaunchSystemUpdateGpuAsync.template operator() < SpotLightSystem > ();

	for (auto& [_, future] : futures) {
		if (future.valid())
			future.get();
	}

	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<System>(timer.GetElapsedTime());
}

void Scene::UpdateComponentBuffers(uint32_t frameIndex)
{
	RecalculateGpuBufferSize<TransformComponent, TransformComponentGPU>("TransformData", frameIndex);
	RecalculateGpuBufferSize<CameraComponent, CameraComponentGPU>("CameraData", frameIndex);
	RecalculateGpuBufferSize<CameraComponent, CameraFrustumGPU>("CameraFrustumData", frameIndex);

	RecalculateGpuBufferSize<MaterialComponent, uint32_t>("MaterialData", frameIndex);
	RecalculateGpuBufferSize<ShapeComponent, ShapeRenderIndicesGPU>("ShapeRenderIndicesData", frameIndex);
	RecalculateGpuBufferSize<ModelComponent, ModelRenderIndicesGPU>("ModelRenderIndicesData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderAabbData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderObbData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderSphereData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, DefaultColliderGPU>("DefaultColliderData", frameIndex);
	RecalculateGpuBufferSize<AnimationComponent, VkDeviceAddress>("AnimationNodeTransformDeviceAddressesBuffers", frameIndex);
	
	RecalculateGpuBufferSize<DirectionLightComponent, DirectionLightGPU>("DirectionLightData", frameIndex);
	RecalculateGpuBufferSize<DirectionLightComponent, glm::vec4>("DirectionLightBillboard", frameIndex);

	RecalculateGpuBufferSize<PointLightComponent, PointLightGPU>("PointLightData", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, glm::mat4>("PointLightTransform", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, uint32_t>("PointLightInstanceIndices", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, uint32_t>("PointLightOcclusionIndices", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, glm::vec4>("PointLightBillboard", frameIndex);

	RecalculateGpuBufferSize<SpotLightComponent, SpotLightGPU>("SpotLightData", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, glm::mat4>("SpotLightTransform", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, uint32_t>("SpotLightInstanceIndices", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, uint32_t>("SpotLightOcclusionIndices", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, glm::vec4>("SpotLightBillboard", frameIndex);	
}