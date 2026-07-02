#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Vk/Buffer/BufferFactory.h"

#include <chrono>
#include <taskflow/taskflow.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include "Settings/SceneSettings.h"
#include "DrawData/SceneDrawData.h"
#include "Engine/Scene/Source/ISceneSource.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "HierarchyManager.h"
#include "Engine/System/SystemContext.h"

namespace Syn
{
    class ISystem;

    enum class SystemPhase
    {
        Update,
        UploadGPU,
        Finish
    };

    class SYN_API Scene
    {
    public:
        Scene(uint32_t frameCount, std::unique_ptr<ISceneSource> source = nullptr, bool initSystems = true);
        virtual ~Scene();

        void Update(float deltaTime, uint32_t frameIndex);
        void UpdateGPU(uint32_t frameIndex);
        void Finish();

        EntityID CreateEntity();
        void DestroyEntity(EntityID entity);

        Registry* GetRegistry() const { return _registry.get(); }
        SceneDrawData* GetSceneDrawData() const { return _sceneDrawData.get(); }
        EntityID GetSceneCameraEntity() const { return _sceneCameraEntity; }
        EntityID GetDebugCameraEntity() const { return _debugCameraEntity; }
        ComponentBufferManager* GetComponentBufferManager() const { return _componentBufferManager.get(); }
        SceneSettings* GetSettings() const { return _sceneSettings.get(); }
		IPhysicsEngine* GetPhysicsEngine() const { return _physicsEngine.get(); }
        HierarchyManager* GetHierarchyManager() const { return _hierarchyManager.get(); }

		EntityID GetSelectedEntity() const { return _selectedEntity; }
		void SetSelectedEntity(EntityID entity) { _selectedEntity = entity; }

        const SystemContext& GetSystemContext() const { return _systemContext; }
    private:
        void InitializeSystems();
        void InitializeComponentBuffers();
        void BuildTaskflowGraph(tf::Taskflow& taskflow, SystemPhase phase);
        void UpdateComponentBuffers(uint32_t frameIndex);

        template<typename T>
        void RegisterSystem();

        template<typename TGpuStruct>
        void RegisterGenericBuffer(const std::string& name, std::function<uint32_t()> sizeCallback, std::function<bool()> activeCallback, ComponentMemoryType memoryType = ComponentMemoryType::Persistent);

        template<typename TComponent, typename TGpuStruct>
        void RegisterComponentBuffer(const std::string& name, ComponentMemoryType memoryType = ComponentMemoryType::Persistent);

        template<typename TComponent>
        void RegisterComponentSparseMapBuffer(const std::string& name);
    protected:
        EntityID _sceneCameraEntity = NULL_ENTITY;
        EntityID _debugCameraEntity = NULL_ENTITY;
        EntityID _selectedEntity = NULL_ENTITY;

        std::unique_ptr<ComponentBufferManager> _componentBufferManager;
        std::vector<std::unique_ptr<ISystem>> _systems;
        std::unique_ptr<SceneDrawData> _sceneDrawData;
        std::unique_ptr<HierarchyManager> _hierarchyManager;

        std::unique_ptr<Registry> _registry;
        std::unique_ptr<SceneSettings> _sceneSettings;
        std::unique_ptr<IPhysicsEngine> _physicsEngine;

        tf::Taskflow _updateTaskflow;
        tf::Taskflow _gpuTaskflow;
        tf::Taskflow _finishTaskflow;

        float _currentDeltaTime = 0.0f;
        uint32_t _currentFrameIndex = 0;

        SystemContext _systemContext;
    private:
        friend class SceneInsider;
    };

    template<typename T>
    SYN_INLINE void Scene::RegisterSystem()
    {
        static_assert(std::is_base_of_v<ISystem, T>, "T must be derived from ISystem");
        _systems.push_back(std::make_unique<T>());
    }

    template<typename TGpuStruct>
    SYN_INLINE void Scene::RegisterGenericBuffer(const std::string& name, std::function<uint32_t()> sizeCallback, std::function<bool()> activeCallback, ComponentMemoryType memoryType)
    {
        _componentBufferManager->RegisterBuffer(name, sizeof(TGpuStruct), std::move(sizeCallback), std::move(activeCallback), memoryType);
    }

    template<typename TComponent, typename TGpuStruct>
    SYN_INLINE void Scene::RegisterComponentBuffer(const std::string& name, ComponentMemoryType memoryType)
    {
        this->RegisterGenericBuffer<TGpuStruct>(name,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TComponent>();
                return pool ? static_cast<uint32_t>(pool->Size()) : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TComponent>();
                return pool && pool->Size() > 0;
            },
            memoryType);
    }

    template<typename TComponent>
    SYN_INLINE void Scene::RegisterComponentSparseMapBuffer(const std::string& name)
    {
        this->RegisterGenericBuffer<DenseIndex>(name,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TComponent>();
                return pool ? static_cast<uint32_t>(pool->GetSparseIndices().size()) : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TComponent>();
                return pool && pool->Size() > 0;
            });
    }
}