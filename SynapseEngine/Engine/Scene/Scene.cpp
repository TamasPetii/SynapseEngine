#include "Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Mesh/ModelManager.h"
#include "BufferNames.h"

#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Component/AnimationComponent.h"

#include "Engine/System/TransformSystem.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/System/MaterialSystem.h"
#include "Engine/System/ModelFrustumCullingSystem.h"
#include "Engine/System/AnimationSystem.h"
#include "Engine/System/PhysicsSystem.h"
#include "Engine/System/PointLightSystem.h"
#include "Engine/System/PointLightShadowSystem.h"
#include "Engine/System/PointLightFrustumCullingSystem.h"

namespace Syn
{
    Scene::Scene(uint32_t frameCount)
    {
        _registry = std::make_unique<Registry>();
        _componentBufferManager = std::make_unique<ComponentBufferManager>(frameCount);
        _sceneDrawData = std::make_unique<SceneDrawData>(frameCount);

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
        RegisterSystem<MaterialSystem>();
        RegisterSystem<CameraSystem>();
        RegisterSystem<RenderSystem>();
        RegisterSystem<ModelSystem>();
        RegisterSystem<ModelFrustumCullingSystem>();
        RegisterSystem<AnimationSystem>();
        RegisterSystem<PointLightSystem>();
        RegisterSystem<PointLightShadowSystem>();
        RegisterSystem<PointLightFrustumCullingSystem>();

        //RegisterSystem<PhysicsSystem>();
    }

    void Scene::InitializeComponentBuffers()
    {
        RegisterComponentSparseMapBuffer<TransformComponent>(BufferNames::TransformSparseMap);
        RegisterComponentBuffer<TransformComponent, TransformComponentGPU>(BufferNames::TransformData);   

        RegisterComponentSparseMapBuffer<CameraComponent>(BufferNames::CameraSparseMap);
        RegisterComponentBuffer<CameraComponent, CameraComponentGPU>(BufferNames::CameraData);

        RegisterComponentSparseMapBuffer<ModelComponent>(BufferNames::ModelSparseMap);
        RegisterComponentBuffer<ModelComponent, ModelComponentGPU>(BufferNames::ModelData);
        RegisterComponentBuffer<ModelComponent, VisibleModelData>(BufferNames::ModelVisibleData);

        RegisterComponentSparseMapBuffer<AnimationComponent>(BufferNames::AnimationSparseMap);
        RegisterComponentBuffer<AnimationComponent, AnimationComponentGPU>(BufferNames::AnimationData);

        RegisterComponentSparseMapBuffer<PointLightComponent>(BufferNames::PointLightSparseMap);
        RegisterComponentBuffer<PointLightComponent, PointLightComponentGPU>(BufferNames::PointLightData);
        RegisterComponentBuffer<PointLightComponent, PointLightColliderGPU>(BufferNames::PointLightColliderData);
        RegisterComponentBuffer<PointLightComponent, uint32_t>(BufferNames::PointLightVisibleData);

        RegisterComponentSparseMapBuffer<PointLightShadowComponent>(BufferNames::PointLightShadowSparseMap);
        RegisterComponentBuffer<PointLightShadowComponent, PointLightShadowComponentGPU>(BufferNames::PointLightShadowData);

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
                        if (readerTask != sysTask)
                        {
                            sysTask.succeed(readerTask);
                        }
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
}