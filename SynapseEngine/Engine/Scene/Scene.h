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

        Scene(uint32_t frameCount);
        virtual ~Scene();

        void Update(float deltaTime, uint32_t frameIndex);
        void UpdateGPU(uint32_t frameIndex);
        void Finish();

        std::shared_ptr<Registry> GetRegistry() const { return _registry; }
        std::shared_ptr<ComponentBufferManager> GetComponentBufferManager() const { return _componentBufferManager; }
        std::shared_ptr<Vk::Buffer> GetGlobalInstanceBuffer() const { return globalInstanceBuffer; }
        std::shared_ptr<Vk::Buffer> GetGlobalIndirectCommandBuffer() const { return globalIndirectCommandBuffer; }
        std::shared_ptr<Vk::Buffer> GetGlobalDescriptorBuffer() const { return globalIndirectCommandDescriptorBuffer; }
        std::shared_ptr<Vk::Buffer> GetGlobalDrawCountBuffer() const { return globalDrawCountBuffer; }
        std::shared_ptr<Vk::Buffer> GetGlobalModelAllocationBuffer() const { return globalModelAllocationBuffer; }
        std::shared_ptr<Vk::Buffer> GetGlobalMeshAllocationBuffer() const { return globalMeshAllocationBuffer; }
    private:
        void InitializeSystems();
        void InitializeComponentBuffers();
        void InitializeGlobalBuffers();
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
        std::shared_ptr<Registry> _registry;
        std::vector<std::shared_ptr<ISystem>> _systems;
        std::shared_ptr<ComponentBufferManager> _componentBufferManager;

        tf::Taskflow _updateTaskflow;
        tf::Taskflow _gpuTaskflow;
        tf::Taskflow _finishTaskflow;

        std::shared_ptr<Vk::Buffer> globalInstanceBuffer;
        std::shared_ptr<Vk::Buffer> globalIndirectCommandBuffer;
        std::shared_ptr<Vk::Buffer> globalIndirectCommandDescriptorBuffer;
        std::shared_ptr<Vk::Buffer> globalDrawCountBuffer;
        std::shared_ptr<Vk::Buffer> globalModelAllocationBuffer;
        std::shared_ptr<Vk::Buffer> globalMeshAllocationBuffer;
        
        std::vector<ModelAllocationInfo> _modelAllocations;
        std::vector<MeshAllocationInfo> _meshAllocations;
        std::vector<VkDrawIndirectCommand> _traditionalCommands;
        std::vector<VkDrawMeshTasksIndirectCommandEXT> _meshletCommands;
        std::vector<uint32_t> _cpuInstanceBuffer;

        float _currentDeltaTime = 0.0f;
        uint32_t _currentFrameIndex = 0;
    };

    template<typename T>
    SYN_INLINE void Scene::RegisterSystem()
    {
        static_assert(std::is_base_of_v<ISystem, T>, "T must be derived from ISystem");
        _systems.push_back(std::make_shared<T>());
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