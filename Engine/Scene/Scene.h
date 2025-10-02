#pragma once
#include "Engine/Registry/Registry.h"
#include "Engine/Systems/Systems.h"
#include "Engine/Components/Components.h"
#include "Engine/Managers/ResourceManager.h"
#include "Engine/Timer/Timer.h"

class ENGINE_API Scene
{
public:
	Scene(std::shared_ptr<ResourceManager> resourceManager);
	~Scene();
	void Update(std::shared_ptr<Timer> frameTimer, uint32_t frameIndex);
	void UpdateGPU(uint32_t frameIndex);
	void Finish();
	std::shared_ptr<Registry> GetRegistry() { return registry; }
private:
	void Initialize();
	void Cleanup();
	void InitializeSystems();
	void InitializeRegistry();
private:
	void UpdateSystems(uint32_t frameIndex, float deltaTime);
	void FinishSystems();
	void UpdateSystemsGPU(uint32_t frameIndex);
	void UpdateComponentBuffers(uint32_t frameIndex);
	template<typename T>
	void InitSystem();
	template<typename T>
	void UpdateSystem(uint32_t frameIndex, float deltaTime);
	template<typename T>
	void FinishSystem();
	template<typename T>
	void UpdateGpuSystem(uint32_t frameIndex);
	template<typename T, typename G>
	void RecalculateGpuBufferSize(const std::string& name, uint32_t frameIndex);
private:
	Entity sceneCameraEntity = NULL_ENTITY;
	std::shared_ptr<Registry> registry;
	std::shared_ptr<ResourceManager> resourceManager;
	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;
};

template<typename T>
inline void Scene::InitSystem()
{
	static_assert(std::is_base_of_v<System, T>, "T must be derived from System");
	systems[Unique::typeID<T>()] = std::make_shared<T>(); 
	resourceManager->GetBenchmarkManager()->Register<T>(); //Register systems for async parallel update -> No async insert into std::unordered_map
}

template<typename T>
inline void Scene::UpdateSystem(uint32_t frameIndex, float deltaTime)
{
	static_assert(std::is_base_of_v<System, T>, "T must be derived from System");
	Timer timer{};
	systems.at(Unique::typeID<T>())->OnUpdate(registry, resourceManager, frameIndex, deltaTime);
	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<T>(timer.GetElapsedTime());
}

template<typename T>
inline void Scene::FinishSystem()
{
	static_assert(std::is_base_of_v<System, T>, "T must be derived from System");
	Timer timer;
	systems.at(Unique::typeID<T>())->OnFinish(registry);
	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<T>(timer.GetElapsedTime());
}

template<typename T>
inline void Scene::UpdateGpuSystem(uint32_t frameIndex)
{
	static_assert(std::is_base_of_v<System, T>, "T must be derived from System");
	Timer timer;
	systems.at(Unique::typeID<T>())->OnUploadToGpu(registry, resourceManager, frameIndex);
	resourceManager->GetBenchmarkManager()->AddBenchmarkTime<T>(timer.GetElapsedTime());
}

template<typename T, typename G>
inline void Scene::RecalculateGpuBufferSize(const std::string& name, uint32_t frameIndex)
{
	static_assert(std::is_base_of_v<Component, T>, "T must be derived from Component");

	static Vk::BufferConfig config = {
		.size = 1,
		.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT,
		.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 
	};

	auto componentBufferManager = resourceManager->GetComponentBufferManager();
	auto pool = registry->GetPool<T>();
	componentBufferManager->RegisterBuffer(name, config);
	componentBufferManager->RecreateBuffer<G>(name, pool ? pool->GetDenseSize() : 1, frameIndex);
}
