#include "Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Mesh/ModelManager.h"
#include "BufferNames.h"

#include "Engine/Scene/Source/ISceneSource.h"

#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"

#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Rendering/ModelFrustumCullingSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Physics/PhysicsSystem.h"
#include "Engine/System/Light/Point/PointLightSystem.h"
#include "Engine/System/Light/Point/PointLightShadowSystem.h"
#include "Engine/System/Light/Point/PointLightFrustumCullingSystem.h"
#include "Engine/System/Light/Spot/SpotLightSystem.h"
#include "Engine/System/Light/Spot/SpotLightShadowSystem.h"
#include "Engine/System/Light/Spot/SpotLightFrustumCullingSystem.h"
#include "Engine/System/Light/Direction/DirectionLightSystem.h"
#include "Engine/System/Light/Direction/DirectionLightShadowSystem.h"
#include "Engine/System/Light/Direction/DirectionLightCullingSystem.h"
#include "Engine/System/Physics/BoxColliderSystem.h"
#include "Engine/System/Physics/SphereColliderSystem.h"
#include "Engine/System/Physics/CapsuleColliderSystem.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"
#include "Engine/System/Core/TransformModelLinkSystem.h"
#include "Engine/Profiler/ICpuProfiler.h"
#include "Engine/FrameContext.h"

namespace Syn
{
    Scene::Scene(uint32_t frameCount, std::unique_ptr<ISceneSource> source, bool initSystems)
    {
        _registry = std::make_unique<Registry>();
        _sceneSettings = std::make_unique<SceneSettings>();

		_registry->EnsurePool<TransformComponent>();
        _registry->EnsurePool<CameraComponent>();
        _registry->EnsurePool<TagComponent>();
        _registry->EnsurePool<AnimationComponent>();
        _registry->EnsurePool<ModelComponent>();
        _registry->EnsurePool<MaterialOverrideComponent>();
        _registry->EnsurePool<DirectionLightComponent>();
        _registry->EnsurePool<DirectionLightShadowComponent>();
        _registry->EnsurePool<PointLightComponent>();
        _registry->EnsurePool<PointLightShadowComponent>();
        _registry->EnsurePool<SpotLightComponent>();
        _registry->EnsurePool<SpotLightShadowComponent>();
		_registry->EnsurePool<BoxColliderComponent>();
		_registry->EnsurePool<SphereColliderComponent>();
		_registry->EnsurePool<CapsuleColliderComponent>();
		_registry->EnsurePool<RigidBodyComponent>();

        _physicsEngine = ServiceLocator::GetPhysicsFactory()();

        if(source)
			source->Populate(*this);

        if (initSystems)
        {
            _componentBufferManager = std::make_unique<ComponentBufferManager>(frameCount);
            _sceneDrawData = std::make_unique<SceneDrawData>(frameCount);

            InitializeSystems();
            InitializeComponentBuffers();

            BuildTaskflowGraph(_updateTaskflow, SystemPhase::Update);
            BuildTaskflowGraph(_gpuTaskflow, SystemPhase::UploadGPU);
            BuildTaskflowGraph(_finishTaskflow, SystemPhase::Finish);
        }
    }

    Scene::~Scene()
    {
		_physicsEngine->Shutdown();
        _physicsEngine.reset();
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
        RegisterSystem<TransformModelLinkSystem>();
        RegisterSystem<StaticSpatialSahSystem>();
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
        RegisterSystem<PhysicsSystem>();
		RegisterSystem<BoxColliderSystem>();
		RegisterSystem<SphereColliderSystem>();
		RegisterSystem<CapsuleColliderSystem>();
		RegisterSystem<RigidBodySystem>();
    }

    void Scene::InitializeComponentBuffers()
    {   
        auto mortonCondition = [this]() -> bool {
            auto pool = _registry->GetPool<TransformComponent>();
            return pool && !pool->GetStorage().GetStaticEntities().empty();
            };

        auto mortonBufferSizing = [this]() -> uint32_t {
            auto pool = _registry->GetPool<TransformComponent>();
            if (!pool) return 0;
            uint32_t staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
            return ((staticCount + 1023) / 1024) * 1024;
            };

        auto mortonChunkBufferSizing = [this]() -> uint32_t {
            auto pool = _registry->GetPool<TransformComponent>();
            if (!pool) return 0;
            uint32_t staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
            return ComputeGroupSize::CalculateDispatchCount(staticCount, ComputeGroupSize::Buffer32D); // Ahol a Buffer32D a CHUNK_SIZE
            };

        RegisterGenericBuffer<uint32_t>(BufferNames::MortonKeysData, mortonBufferSizing, mortonCondition, ComponentMemoryType::GpuOnly);
        RegisterGenericBuffer<uint32_t>(BufferNames::MortonValuesData, mortonBufferSizing, mortonCondition, ComponentMemoryType::GpuOnly);
        RegisterGenericBuffer<ChunkDataGPU>(BufferNames::MortonChunkData, mortonChunkBufferSizing, mortonCondition, ComponentMemoryType::GpuOnly);
        RegisterGenericBuffer<uint32_t>(BufferNames::MortonChunkVisibileIndex, mortonChunkBufferSizing, mortonCondition, ComponentMemoryType::GpuOnly);
        RegisterComponentBuffer<TransformComponent, uint32_t>(BufferNames::MortonChunkTransformsIndex, ComponentMemoryType::GpuOnly);

        
        RegisterGenericBuffer<ChunkDataGPU>(BufferNames::MortonChunkData,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool ? ComputeGroupSize::CalculateDispatchCount(static_cast<uint32_t>(pool->Size()), ComputeGroupSize::Buffer32D) : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<uint32_t>(BufferNames::MortonChunkVisibileIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool ? ComputeGroupSize::CalculateDispatchCount(static_cast<uint32_t>(pool->Size()), ComputeGroupSize::Buffer32D) : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterComponentSparseMapBuffer<TransformComponent>(BufferNames::TransformSparseMap);
        RegisterComponentBuffer<TransformComponent, TransformComponentGPU>(BufferNames::TransformData);   
        RegisterComponentBuffer<TransformComponent, TransformModelLinkGPU>(BufferNames::TransformModelLinkData);

        RegisterComponentSparseMapBuffer<CameraComponent>(BufferNames::CameraSparseMap);
        RegisterComponentBuffer<CameraComponent, CameraComponentGPU>(BufferNames::CameraData);
        RegisterComponentBuffer<CameraComponent, uint32_t>(BufferNames::CameraVisibleData);

        RegisterComponentSparseMapBuffer<ModelComponent>(BufferNames::ModelSparseMap);
        RegisterComponentBuffer<ModelComponent, ModelComponentGPU>(BufferNames::ModelData);
        RegisterComponentBuffer<ModelComponent, VisibleModelData>(BufferNames::ModelVisibleData, ComponentMemoryType::GpuOnly);

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

		RegisterComponentSparseMapBuffer<BoxColliderComponent>(BufferNames::BoxColliderSparseMap);
		RegisterComponentBuffer<BoxColliderComponent, BoxColliderComponentGPU>(BufferNames::BoxColliderData);

		RegisterComponentSparseMapBuffer<SphereColliderComponent>(BufferNames::SphereColliderSparseMap);
		RegisterComponentBuffer<SphereColliderComponent, SphereColliderComponentGPU>(BufferNames::SphereColliderData);

		RegisterComponentSparseMapBuffer<CapsuleColliderComponent>(BufferNames::CapsuleColliderSparseMap);
		RegisterComponentBuffer<CapsuleColliderComponent, CapsuleColliderComponentGPU>(BufferNames::CapsuleColliderData);
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

                std::string groupName = sys->GetGroup();

                CpuProfileScope profile(ServiceLocator::GetCpuProfiler(), _currentFrameIndex, groupName, profilerName);

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

                subflow.join();

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
            _registry->GetComponent<CameraComponent>(_sceneCameraEntity).width = (float)screenWidth;
            _registry->GetComponent<CameraComponent>(_sceneCameraEntity).height = (float)screenHeight;
        }

        if (_debugCameraEntity != NULL_ENTITY && _registry->HasComponent<CameraComponent>(_debugCameraEntity))
        {
            _registry->GetComponent<CameraComponent>(_debugCameraEntity).width = (float)screenWidth;
            _registry->GetComponent<CameraComponent>(_debugCameraEntity).height = (float)screenHeight;
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