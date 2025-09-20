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

	/*
	std::string imagePath = "../Assets/Texture.jpg";
	resourceManager->GetImageManager()->LoadImage(imagePath, false);
	resourceManager->GetImageManager()->WaitForImageFuture(imagePath);
	*/

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
			registry->SetParent(entity, directionLightParent);
		}

		//Point Lights
		{
			for (int i = 0; i < 100; ++i)
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, PointLightComponent>(entity);
				registry->GetComponent<TransformComponent>(entity).translation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 50.f;
				registry->GetComponent<PointLightComponent>(entity).color = glm::vec3(dist(rng), dist(rng), dist(rng));
				registry->SetParent(entity, pointLightParent);
			}
		}

		//Spot Lights
		{
			for (int i = 0; i < 100; ++i)
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, SpotLightComponent>(entity);
				registry->GetComponent<TransformComponent>(entity).translation = glm::vec3(dist(rng), dist(rng), dist(rng)) * 50.f;
				registry->GetComponent<TransformComponent>(entity).scale = glm::vec3(5.f, 25.f, 25.f);
				registry->GetComponent<SpotLightComponent>(entity).color = glm::vec3(dist(rng), dist(rng), dist(rng));
				registry->SetParent(entity, spotLightParent);
			}
		}
		
	}

	//Shapes
	{
		auto shapeParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(shapeParent);

		for (uint32_t x = 0; x < 10; ++x)
		{
			for (uint32_t y = 0; y < 10; ++y)
			{
				auto entity = registry->CreateEntity();
				registry->AddComponents<TransformComponent, MaterialComponent, ShapeComponent, DefaultColliderComponent>(entity);

				auto [transformComponent, materialComponent, shapeComponent] = registry->GetComponents<TransformComponent, MaterialComponent, ShapeComponent>(entity);
				transformComponent.translation = 2.f * glm::vec3(x, y, 0);

				materialComponent.color = glm::vec4(dist(rng), dist(rng), dist(rng), 1);
				materialComponent.roughness = x / 10.f;
				materialComponent.metalness = y / 10.f;
			
				shapeComponent.shape = resourceManager->GetGeometryManager()->GetShape("Sphere");

				registry->SetParent(entity, shapeParent);
			}
		}
	}

	//Models
	{
		auto modelParent = registry->CreateEntity();
		registry->AddComponents<TransformComponent>(modelParent);

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

		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent] = registry->GetComponents<TransformComponent, ModelComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
			transformComponent.translation = 100.f * glm::vec3(dist(rng), 0, dist(rng));

			registry->SetParent(entity, animationParent);
		}

		for (uint32_t i = 0; i < 25; ++i)
		{
			auto entity = registry->CreateEntity();
			registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
			auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
			modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
			animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation(animationPaths[distAnimation(rng)]);
			transformComponent.translation = 100.f * glm::vec3(dist(rng), 0, dist(rng));

			registry->SetParent(entity, animationParent);
		}

		/*
	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
		auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
		modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
		animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
	}

	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
		auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
		modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
		animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation("C:/Users/User/Desktop/Animations/Worker_Walking.dae");
	}

	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
		auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
		modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
		animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation("C:/Users/User/Desktop/Animations/Worker_Running.dae");
	}

	{
		auto entity = registry->CreateEntity();
		registry->AddComponents<TransformComponent, ModelComponent, AnimationComponent, DefaultColliderComponent>(entity);
		auto [transformComponent, modelComponent, animationComponent] = registry->GetComponents<TransformComponent, ModelComponent, AnimationComponent>(entity);
		modelComponent.model = resourceManager->GetModelManager()->LoadModel("C:/Users/User/Desktop/Animations/Worker_Standing.dae");
		animationComponent.animation = resourceManager->GetAnimationManager()->LoadAnimation("C:/Users/User/Desktop/Animations/Worker_Dancing.dae");
	}
	*/
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
	RecalculateGpuBufferSize<MaterialComponent, MaterialComponentGPU>("MaterialData", frameIndex);
	RecalculateGpuBufferSize<ShapeComponent, ShapeRenderIndicesGPU>("ShapeRenderIndicesData", frameIndex);
	RecalculateGpuBufferSize<ModelComponent, ModelRenderIndicesGPU>("ModelRenderIndicesData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderAabbData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderObbData", frameIndex);
	RecalculateGpuBufferSize<DefaultColliderComponent, glm::mat4>("DefaultColliderSphereData", frameIndex);
	RecalculateGpuBufferSize<AnimationComponent, VkDeviceAddress>("AnimationNodeTransformDeviceAddressesBuffers", frameIndex);
	RecalculateGpuBufferSize<DirectionLightComponent, DirectionLightGPU>("DirectionLightData", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, PointLightGPU>("PointLightData", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, glm::mat4>("PointLightTransform", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, uint32_t>("PointLightInstanceIndices", frameIndex);
	RecalculateGpuBufferSize<PointLightComponent, uint32_t>("PointLightOcclusionIndices", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, SpotLightGPU>("SpotLightData", frameIndex);
	RecalculateGpuBufferSize<SpotLightComponent, glm::mat4>("SpotLightTransform", frameIndex);
}