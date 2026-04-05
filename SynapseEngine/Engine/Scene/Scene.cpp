#include "Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Mesh/ModelManager.h"
#include "BufferNames.h"

#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Component/ModelComponent.h"
#include "Engine/Component/AnimationComponent.h"
#include "Engine/Component/DirectionLightComponent.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Component/SpotLightComponent.h"

#include "Engine/System/TransformSystem.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/System/CameraSystem.h"
#include "Engine/System/ModelSystem.h"
#include "Engine/System/MaterialSystem.h"
#include "Engine/System/ModelFrustumCullingSystem.h"
#include "Engine/System/AnimationSystem.h"
#include "Engine/System/PhysicsSystem.h"
#include "Engine/System/Light/PointLightSystem.h"
#include "Engine/System/Light/PointLightShadowSystem.h"
#include "Engine/System/Light/PointLightFrustumCullingSystem.h"
#include "Engine/System/Light/SpotLightSystem.h"
#include "Engine/System/Light/SpotLightShadowSystem.h"
#include "Engine/System/Light/SpotLightFrustumCullingSystem.h"
#include "Engine/System/Light/DirectionLightSystem.h"
#include "Engine/System/Light/DirectionLightShadowSystem.h"
#include "Engine/System/Light/DirectionLightCullingSystem.h"
#include "Engine/Component/MaterialOverrideComponent.h"
#include "Engine/Profiler/ICpuProfiler.h"

#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"

namespace Syn
{
    Scene::Scene(uint32_t frameCount)
    {
        _registry = std::make_unique<Registry>();
		_registry->EnsurePool<TransformComponent>();
        _registry->EnsurePool<AnimationComponent>();
        _registry->EnsurePool<CameraComponent>();
        _registry->EnsurePool<ModelComponent>();
        _registry->EnsurePool<MaterialOverrideComponent>();
        _registry->EnsurePool<DirectionLightComponent>();
        _registry->EnsurePool<DirectionLightShadowComponent>();
        _registry->EnsurePool<PointLightComponent>();
        _registry->EnsurePool<PointLightShadowComponent>();
        _registry->EnsurePool<SpotLightComponent>();
        _registry->EnsurePool<SpotLightShadowComponent>();

        _componentBufferManager = std::make_unique<ComponentBufferManager>(frameCount);
        _sceneDrawData = std::make_unique<SceneDrawData>(frameCount);
        _sceneSettings = std::make_unique<SceneSettings>();

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
        RegisterSystem<SpotLightSystem>();
        RegisterSystem<SpotLightShadowSystem>();
        RegisterSystem<SpotLightFrustumCullingSystem>();
		RegisterSystem<DirectionLightSystem>();
        RegisterSystem<DirectionLightCullingSystem>();
        RegisterSystem<DirectionLightShadowSystem>();
        //RegisterSystem<PhysicsSystem>();
    }

    void Scene::InitializeComponentBuffers()
    {
        RegisterComponentSparseMapBuffer<TransformComponent>(BufferNames::TransformSparseMap);
        RegisterComponentBuffer<TransformComponent, TransformComponentGPU>(BufferNames::TransformData);   

        RegisterComponentSparseMapBuffer<CameraComponent>(BufferNames::CameraSparseMap);
        RegisterComponentBuffer<CameraComponent, CameraComponentGPU>(BufferNames::CameraData);
        RegisterComponentBuffer<CameraComponent, uint32_t>(BufferNames::CameraVisibleData);

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

        RegisterComponentSparseMapBuffer<SpotLightComponent>(BufferNames::SpotLightSparseMap);
        RegisterComponentBuffer<SpotLightComponent, SpotLightComponentGPU>(BufferNames::SpotLightData);
        RegisterComponentBuffer<SpotLightComponent, SpotLightColliderGPU>(BufferNames::SpotLightColliderData);
        RegisterComponentBuffer<SpotLightComponent, uint32_t>(BufferNames::SpotLightVisibleData);

        RegisterComponentSparseMapBuffer<SpotLightShadowComponent>(BufferNames::SpotLightShadowSparseMap);
        RegisterComponentBuffer<SpotLightShadowComponent, SpotLightShadowComponentGPU>(BufferNames::SpotLightShadowData);

        RegisterComponentSparseMapBuffer<DirectionLightComponent>(BufferNames::DirectionLightSparseMap);
        RegisterComponentBuffer<DirectionLightComponent, DirectionLightComponentGPU>(BufferNames::DirectionLightData);
        RegisterComponentBuffer<DirectionLightComponent, uint32_t>(BufferNames::DirectionLightVisibleData);

        RegisterComponentSparseMapBuffer<DirectionLightShadowComponent>(BufferNames::DirectionLightShadowSparseMap);
        RegisterComponentBuffer<DirectionLightShadowComponent, DirectionLightShadowGPU>(BufferNames::DirectionLightShadowData);
        RegisterComponentBuffer<DirectionLightShadowComponent, DirectionLightShadowColliderGPU>(BufferNames::DirectionLightShadowColliderData);
    }

    void Scene::BuildTaskflowGraph(tf::Taskflow& taskflow, SystemPhase phase)
    {
        taskflow.clear();

        std::unordered_map<TypeID, tf::Task> lastWriters;
        std::unordered_map<TypeID, std::vector<tf::Task>> lastReaders;

        for (auto& system : _systems)
        {
            tf::Task sysTask = taskflow.emplace([sys = system.get(), phase, this](tf::Subflow& subflow) {

                std::string profilerName = sys->GetName();
                if (phase == SystemPhase::Update) profilerName += " [Update]";
                else if (phase == SystemPhase::UploadGPU) profilerName += " [Upload GPU]";
                else profilerName += " [Finish]";

                CpuProfileScope profile(ServiceLocator::GetCpuProfiler(), _currentFrameIndex, profilerName);

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

        auto screenWidth = ServiceLocator::GetFrameContext()->screenWidth;
        auto screenHeight = ServiceLocator::GetFrameContext()->screenHeight;

        if (_sceneCameraEntity != NULL_ENTITY && _registry->HasComponent<CameraComponent>(_sceneCameraEntity))
        {
            _registry->GetComponent<CameraComponent>(_sceneCameraEntity).width = screenWidth;
            _registry->GetComponent<CameraComponent>(_sceneCameraEntity).height = screenHeight;
        }

        if (_debugCameraEntity != NULL_ENTITY && _registry->HasComponent<CameraComponent>(_debugCameraEntity))
        {
            _registry->GetComponent<CameraComponent>(_debugCameraEntity).width = screenWidth;
            _registry->GetComponent<CameraComponent>(_debugCameraEntity).height = screenHeight;
        }

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