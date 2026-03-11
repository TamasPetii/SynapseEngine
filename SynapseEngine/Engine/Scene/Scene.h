#pragma once
#include "Engine/SynApi.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include <taskflow/taskflow.hpp>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include "SceneDrawData.h"

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
        static constexpr uint32_t MAX_INSTANCES = 10000000;
        static constexpr uint32_t MAX_INDIRECT_COMMANDS = 200000;
        static constexpr uint32_t MESHLET_OFFSET_START = 100000;

        Scene(uint32_t frameCount);
        virtual ~Scene();

        void Update(float deltaTime, uint32_t frameIndex);
        void UpdateGPU(uint32_t frameIndex);
        void Finish();

        Registry* GetRegistry() const { return _registry.get(); }
        ComponentBufferManager* GetComponentBufferManager() const { return _componentBufferManager.get(); }
        SceneDrawData* GetSceneDrawData() const { return _sceneDrawData.get(); }
    private:
        void InitializeSystems();
        void InitializeComponentBuffers();
        void BuildTaskflowGraph(tf::Taskflow& taskflow, SystemPhase phase);
        void UpdateComponentBuffers(uint32_t frameIndex);

        template<typename T>
        void RegisterSystem();

        template<typename TComponent, typename TGpuStruct>
        void RegisterComponentBuffer(const std::string& name);

        template<typename TComponent>
        void RegisterComponentSparseMapBuffer(const std::string& name);
    protected:
        EntityID _sceneCameraEntity = NULL_ENTITY;

        std::unique_ptr<Registry> _registry;
        std::unique_ptr<SceneDrawData> _sceneDrawData;
        std::unique_ptr<ComponentBufferManager> _componentBufferManager;

        std::vector<std::unique_ptr<ISystem>> _systems;

        tf::Taskflow _updateTaskflow;
        tf::Taskflow _gpuTaskflow;
        tf::Taskflow _finishTaskflow;

        float _currentDeltaTime = 0.0f;
        uint32_t _currentFrameIndex = 0;
    };

    template<typename T>
    SYN_INLINE void Scene::RegisterSystem()
    {
        static_assert(std::is_base_of_v<ISystem, T>, "T must be derived from ISystem");
        _systems.push_back(std::make_unique<T>());
    }

    template<typename TComponent, typename TGpuStruct>
    SYN_INLINE void Scene::RegisterComponentBuffer(const std::string& name)
    {
        _componentBufferManager->RegisterBuffer(name, sizeof(TGpuStruct), [this]() -> uint32_t {
            auto pool = _registry->GetPool<TComponent>();
            return pool ? static_cast<uint32_t>(pool->Size()) : 0;
            });
    }

    template<typename TComponent>
    SYN_INLINE void Scene::RegisterComponentSparseMapBuffer(const std::string& name)
    {
        _componentBufferManager->RegisterBuffer(name, sizeof(DenseIndex), [this]() -> uint32_t {
            auto pool = _registry->GetPool<TComponent>();
            return pool ? static_cast<uint32_t>(pool->GetSparseIndices().size()) : 0;
            });
    }
}