#include "Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "BufferNames.h"

namespace Syn
{
    Scene::Scene(uint32_t frameCount)
    {
        _registry = std::make_shared<Registry>();
        _componentBufferManager = std::make_shared<ComponentBufferManager>(frameCount);

        InitializeSystems();
        InitializeComponentBuffers();
        InitializeGlobalBuffers();

        BuildTaskflowGraph(_updateTaskflow, SystemPhase::Update);
        BuildTaskflowGraph(_gpuTaskflow, SystemPhase::UploadGPU);
        BuildTaskflowGraph(_finishTaskflow, SystemPhase::Finish);
    }

    Scene::~Scene()
    {
        _registry.reset();
        _componentBufferManager.reset();
        _systems.clear();
        _updateTaskflow.clear();
        _gpuTaskflow.clear();
        _finishTaskflow.clear();
    }

    void Scene::InitializeSystems()
    {
        RegisterSystem<TransformSystem>();
        RegisterSystem<CameraSystem>();
        RegisterSystem<RenderSystem>();
    }

    void Scene::InitializeComponentBuffers()
    {
        RegisterComponentSparseMapBuffer<TransformComponent>(BufferNames::TransformSparseMap);
        RegisterComponentBuffer<TransformComponent, TransformComponentGPU>(BufferNames::TransformData);   

        RegisterComponentSparseMapBuffer<CameraComponent>(BufferNames::CameraSparseMap);
        RegisterComponentBuffer<CameraComponent, CameraComponentGPU>(BufferNames::CameraData);
        RegisterComponentBuffer<CameraComponent, CameraFrustumGPU>(BufferNames::CameraFrustumData);
    }

    void Scene::BuildTaskflowGraph(tf::Taskflow& taskflow, SystemPhase phase)
    {
        taskflow.clear();

        std::unordered_map<TypeID, tf::Task> lastWriters;
        std::unordered_map<TypeID, std::vector<tf::Task>> lastReaders;

        for (auto& system : _systems)
        {
            tf::Task sysTask = taskflow.emplace([sys = system.get(), phase, this](tf::Subflow& subflow) {
                switch (phase)
                {
                case SystemPhase::Update:
                    sys->OnUpdate(this, _currentFrameIndex, _currentDeltaTime, subflow);
                    break;
                case SystemPhase::UploadGPU:
                    sys->OnUploadToGpu(this, _currentFrameIndex, subflow);
                    break;
                case SystemPhase::Finish:
                    sys->OnFinish(this, subflow);
                    break;
                }
                }).name(system->GetName());

            if (phase == SystemPhase::Update)
            {
                for (auto typeId : system->GetReadDependencies())
                {
                    if (lastWriters.contains(typeId))
                    {
                        sysTask.succeed(lastWriters[typeId]);
                    }
                    lastReaders[typeId].push_back(sysTask);
                }

                for (auto typeId : system->GetWriteDependencies())
                {
                    if (lastWriters.contains(typeId))
                    {
                        sysTask.succeed(lastWriters[typeId]);
                    }
                    for (auto& readerTask : lastReaders[typeId])
                    {
                        sysTask.succeed(readerTask);
                    }

                    lastReaders[typeId].clear();
                    lastWriters[typeId] = sysTask;
                }
            }      
        }
    }

    void Scene::Update(float deltaTime, uint32_t frameIndex)
    {
        _currentFrameIndex = frameIndex;
        _currentDeltaTime = deltaTime;

        ServiceLocator::GetTaskExecutor()->run(_updateTaskflow).wait();
    }

    void Scene::UpdateGPU(uint32_t frameIndex)
    {
        _currentFrameIndex = frameIndex;
        _componentBufferManager->Update(frameIndex);

        ServiceLocator::GetTaskExecutor()->run(_gpuTaskflow).wait();
    }

    void Scene::Finish()
    {
        ServiceLocator::GetTaskExecutor()->run(_finishTaskflow).wait();
    }

    void Scene::InitializeGlobalBuffers()
    {
        uint32_t maxInstances = 10000000;
        globalInstanceBuffer = Vk::BufferFactory::CreatePersistent(
            maxInstances * sizeof(uint32_t),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
        );

        globalIndirectCommandDescriptorBuffer = Vk::BufferFactory::CreatePersistent(
            RenderSystem::MAX_INDIRECT_COMMANDS * sizeof(MeshDrawDescriptor),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        );

        size_t traditionalBytes = RenderSystem::MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        size_t meshletBytes = (RenderSystem::MAX_INDIRECT_COMMANDS - RenderSystem::MESHLET_OFFSET_START) * sizeof(VkDrawMeshTasksIndirectCommandEXT);

        globalIndirectCommandBuffer = Vk::BufferFactory::CreatePersistent(
            traditionalBytes + meshletBytes,
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        );

        globalDrawCountBuffer = Vk::BufferFactory::CreatePersistent(
            2 * sizeof(uint32_t),
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        );
    }
}