#include "Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/System/TransformSystem.h"
#include "Engine/Component/TransformComponent.h"

namespace Syn
{
    Scene::Scene(uint32_t frameCount)
    {
        _registry = std::make_shared<Registry>();
        _componentBufferManager = std::make_shared<ComponentBufferManager>(frameCount);

        InitializeSystems();
        InitializeComponentBuffers();

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
    }

    void Scene::InitializeComponentBuffers()
    {
        RegisterComponentBuffer<TransformComponent, TransformComponentGPU>("TransformData");
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
                    sys->OnUpdate(_registry, _currentFrameIndex, _currentDeltaTime, subflow);
                    break;
                case SystemPhase::UploadGPU:
                    sys->OnUploadToGpu(_registry, _componentBufferManager, _currentFrameIndex, subflow);
                    break;
                case SystemPhase::Finish:
                    sys->OnFinish(_registry, subflow);
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
        _componentBufferManager->Update(frameIndex); //Todo problematic -> Need to reload previous data??

        ServiceLocator::GetTaskExecutor()->run(_gpuTaskflow).wait();
    }

    void Scene::Finish()
    {
        ServiceLocator::GetTaskExecutor()->run(_finishTaskflow).wait();
    }
}