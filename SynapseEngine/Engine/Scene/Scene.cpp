// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
#include "Engine/Component/Rendering/PipelineOverrideComponent.h"

#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Core/TransformSetupSystem.h"
#include "Engine/System/Core/HierarchySystem.h"
#include "Engine/System/Core/SelectionOutlineSystem.h"
#include "Engine/System/Core/TagSystem.h"
#include "Engine/System/Core/TagSetupSystem.h"
#include "Engine/System/Core/CameraSystem.h"
#include "Engine/System/Rendering/RenderSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Rendering/MaterialSystem.h"
#include "Engine/System/Rendering/ModelFrustumCullingSystem.h"
#include "Engine/System/Rendering/AnimationSystem.h"
#include "Engine/System/Rendering/MaterialOverrideSystem.h"
#include "Engine/System/Rendering/PipelineSystem.h"
#include "Engine/System/Rendering/PipelineOverrideSystem.h"
#include "Engine/System/Physics/PhysicsSystem.h"
#include "Engine/System/Audio/AudioSystem.h"

#include "Engine/System/Light/Point/PointLightSystem.h"
#include "Engine/System/Light/Point/PointLightShadowSystem.h"
#include "Engine/System/Light/Point/PointLightCullingSystem.h"
#include "Engine/System/Light/Point/PointLightShadowRenderSystem.h"
#include "Engine/System/Light/Point/PointLightShadowCullingSystem.h"
#include "Engine/System/Light/Point/PointLightShadowAtlasSystem.h"

#include "Engine/System/Light/Direction/DirectionLightSystem.h"
#include "Engine/System/Light/Direction/DirectionLightShadowSystem.h"
#include "Engine/System/Light/Direction/DirectionLightCullingSystem.h"
#include "Engine/System/Light/Direction/DirectionLightShadowRenderSystem.h"
#include "Engine/System/Light/Direction/DirectionLightShadowCullingSystem.h"
#include "Engine/System/Light/Direction/DirectionLightShadowAtlasSystem.h"

#include "Engine/System/Light/Spot/SpotLightSystem.h"
#include "Engine/System/Light/Spot/SpotLightShadowSystem.h"
#include "Engine/System/Light/Spot/SpotLightCullingSystem.h"
#include "Engine/System/Light/Spot/SpotLightShadowRenderSystem.h"
#include "Engine/System/Light/Spot/SpotLightShadowCullingSystem.h"
#include "Engine/System/Light/Spot/SpotLightShadowAtlasSystem.h"

#include "Engine/System/Physics/BoxColliderSystem.h"
#include "Engine/System/Physics/SphereColliderSystem.h"
#include "Engine/System/Physics/CapsuleColliderSystem.h"
#include "Engine/System/Physics/ConvexColliderSystem.h"
#include "Engine/System/Physics/MeshColliderSystem.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Physics/PhysicsDebugSystem.h"
#include "Engine/System/Core/StaticSpatialSahSystem.h"
#include "Engine/System/Core/TransformModelLinkSystem.h"
#include "Engine/Profiler/ICpuProfiler.h"
#include "Engine/FrameContext.h"

namespace Syn
{
    EntityID Scene::CreateEntity() {
        EntityID entity = _registry->CreateEntity();
        _hierarchyManager->OnEntityCreated(entity);
		_registry->GetPool<HierarchyComponent>()->SetCategory(entity, StorageCategory::Static);
        return entity;
    }

    void Scene::DestroyEntity(EntityID entity) {
        if (!_registry->IsValid(entity)) return;
        
        for (auto& system : _systems) {
            system->OnEntityDestroyed(this, entity);
        }

        _hierarchyManager->OnEntityDestroyed(entity);
        _registry->DestroyEntity(entity);
    }

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
        _registry->EnsurePool<PipelineOverrideComponent>();
        _registry->EnsurePool<DirectionLightComponent>();
        _registry->EnsurePool<DirectionLightShadowComponent>();
        _registry->EnsurePool<PointLightComponent>();
        _registry->EnsurePool<PointLightShadowComponent>();
        _registry->EnsurePool<SpotLightComponent>();
        _registry->EnsurePool<SpotLightShadowComponent>();
		_registry->EnsurePool<BoxColliderComponent>();
		_registry->EnsurePool<SphereColliderComponent>();
		_registry->EnsurePool<CapsuleColliderComponent>();
        _registry->EnsurePool<ConvexColliderComponent>();
        _registry->EnsurePool<MeshColliderComponent>();
		_registry->EnsurePool<RigidBodyComponent>();
		_registry->EnsurePool<HierarchyComponent>();

        _physicsEngine = ServiceLocator::Get<PhysicsFactory>()();

        _hierarchyManager = std::make_unique<HierarchyManager>(_registry.get());

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
        for (auto& system : _systems) {
            system->OnClean(this);
        }

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
        RegisterSystem<TransformSetupSystem>();
        RegisterSystem<TransformSystem>();
        RegisterSystem<TransformModelLinkSystem>();

        RegisterSystem<TagSetupSystem>();
        RegisterSystem<TagSystem>();

        RegisterSystem<StaticSpatialSahSystem>();
        RegisterSystem<MaterialSystem>();
        RegisterSystem<PipelineSystem>();
        RegisterSystem<CameraSystem>();
        RegisterSystem<RenderSystem>();
        RegisterSystem<ModelSystem>();
        RegisterSystem<ModelFrustumCullingSystem>();
        RegisterSystem<AnimationSystem>();
        RegisterSystem<MaterialOverrideSystem>();
        RegisterSystem<PipelineOverrideSystem>();

        RegisterSystem<PointLightSystem>();
        RegisterSystem<PointLightShadowSystem>();
        RegisterSystem<PointLightCullingSystem>();
        RegisterSystem<PointLightShadowRenderSystem>();
        RegisterSystem<PointLightShadowCullingSystem>();
        RegisterSystem<PointLightShadowAtlasSystem>();

        RegisterSystem<SpotLightSystem>();
        RegisterSystem<SpotLightShadowSystem>();
        RegisterSystem<SpotLightCullingSystem>();
        RegisterSystem<SpotLightShadowRenderSystem>();
        RegisterSystem<SpotLightShadowCullingSystem>();
        RegisterSystem<SpotLightShadowAtlasSystem>();

		RegisterSystem<DirectionLightSystem>();
        RegisterSystem<DirectionLightCullingSystem>();
        RegisterSystem<DirectionLightShadowSystem>();
        RegisterSystem<DirectionLightShadowRenderSystem>();
        RegisterSystem<DirectionLightShadowCullingSystem>();
        RegisterSystem<DirectionLightShadowAtlasSystem>();

        RegisterSystem<PhysicsSystem>();
        RegisterSystem<PhysicsDebugSystem>();
		RegisterSystem<BoxColliderSystem>();
		RegisterSystem<SphereColliderSystem>();
		RegisterSystem<CapsuleColliderSystem>();
        RegisterSystem<ConvexColliderSystem>();
        RegisterSystem<MeshColliderSystem>();
		RegisterSystem<RigidBodySystem>();

		RegisterSystem<HierarchySystem>();
        RegisterSystem<SelectionOutlineSystem>();
        RegisterSystem<AudioSystem>();
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

        RegisterComponentSparseMapBuffer<HierarchyComponent>(BufferNames::HierarchySparseMap);
        RegisterComponentBuffer<HierarchyComponent, uint32_t>(BufferNames::SelectionOutlineData);

        RegisterComponentSparseMapBuffer<TagComponent>(BufferNames::TagSparseMap);
        RegisterComponentBuffer<TagComponent, uint32_t>(BufferNames::TagData);

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
        RegisterComponentBuffer<PointLightShadowComponent, uint32_t>(BufferNames::PointLightShadowVisibleData);
        RegisterComponentBuffer<PointLightShadowComponent, uint32_t>(BufferNames::PointLightShadowAtlasSortKeyBuffer);
        RegisterComponentBuffer<PointLightShadowComponent, uint32_t>(BufferNames::PointLightShadowAtlasSortValueBuffer);

        RegisterComponentSparseMapBuffer<SpotLightComponent>(BufferNames::SpotLightSparseMap);
        RegisterComponentBuffer<SpotLightComponent, SpotLightComponentGPU>(BufferNames::SpotLightData);
        RegisterComponentBuffer<SpotLightComponent, SpotLightColliderGPU>(BufferNames::SpotLightColliderData);
        RegisterComponentBuffer<SpotLightComponent, uint32_t>(BufferNames::SpotLightVisibleData);

        RegisterComponentSparseMapBuffer<SpotLightShadowComponent>(BufferNames::SpotLightShadowSparseMap);
        RegisterComponentBuffer<SpotLightShadowComponent, SpotLightShadowComponentGPU>(BufferNames::SpotLightShadowData);
        RegisterComponentBuffer<SpotLightShadowComponent, uint32_t>(BufferNames::SpotLightShadowVisibleData);
        RegisterComponentBuffer<SpotLightShadowComponent, uint32_t>(BufferNames::SpotLightShadowAtlasSortKeyBuffer);
        RegisterComponentBuffer<SpotLightShadowComponent, uint32_t>(BufferNames::SpotLightShadowAtlasSortValueBuffer);

        RegisterComponentSparseMapBuffer<DirectionLightComponent>(BufferNames::DirectionLightSparseMap);
        RegisterComponentBuffer<DirectionLightComponent, DirectionLightComponentGPU>(BufferNames::DirectionLightData);
        RegisterComponentBuffer<DirectionLightComponent, uint32_t>(BufferNames::DirectionLightVisibleData);
        RegisterComponentBuffer<DirectionLightComponent, uint32_t>(BufferNames::DirectionLightVisibleShadowData);

        RegisterComponentSparseMapBuffer<DirectionLightShadowComponent>(BufferNames::DirectionLightShadowSparseMap);
        RegisterComponentBuffer<DirectionLightShadowComponent, DirectionLightShadowGPU>(BufferNames::DirectionLightShadowData);
        RegisterComponentBuffer<DirectionLightShadowComponent, DirectionLightShadowColliderGPU>(BufferNames::DirectionLightShadowColliderData);

		RegisterComponentSparseMapBuffer<BoxColliderComponent>(BufferNames::BoxColliderSparseMap);
		RegisterComponentBuffer<BoxColliderComponent, BoxColliderComponentGPU>(BufferNames::BoxColliderData);

		RegisterComponentSparseMapBuffer<SphereColliderComponent>(BufferNames::SphereColliderSparseMap);
		RegisterComponentBuffer<SphereColliderComponent, SphereColliderComponentGPU>(BufferNames::SphereColliderData);

		RegisterComponentSparseMapBuffer<CapsuleColliderComponent>(BufferNames::CapsuleColliderSparseMap);
		RegisterComponentBuffer<CapsuleColliderComponent, CapsuleColliderComponentGPU>(BufferNames::CapsuleColliderData);

        RegisterComponentSparseMapBuffer<ConvexColliderComponent>(BufferNames::ConvexColliderSparseMap);
        RegisterComponentBuffer<ConvexColliderComponent, ConvexColliderComponentGPU>(BufferNames::ConvexColliderData);

        RegisterComponentSparseMapBuffer<MeshColliderComponent>(BufferNames::MeshColliderSparseMap);
        RegisterComponentBuffer<MeshColliderComponent, MeshColliderComponentGPU>(BufferNames::MeshColliderData);

        RegisterGenericBuffer<VisibleModelData>(BufferNames::DirectionLightShadowModelVisibleData,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool ? static_cast<uint32_t>(pool->Size()) * SHADOW_MULTIPLIER : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<uint32_t>(BufferNames::DirectionLightShadowMortonChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(static_cast<uint32_t>(pool->Size()), ComputeGroupSize::Buffer32D);
                return chunkCount * SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<uint32_t>(BufferNames::DirectionLightShadowStaticChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(staticCount, ComputeGroupSize::Buffer32D);

                return chunkCount * SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && !pool->GetStorage().GetStaticEntities().empty();
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<VisibleModelData>(BufferNames::SpotLightShadowModelVisibleData,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool ? static_cast<uint32_t>(pool->Size()) * SPOT_SHADOW_MULTIPLIER : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<glm::vec2>(BufferNames::SpotLightShadowMortonChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(static_cast<uint32_t>(pool->Size()), ComputeGroupSize::Buffer32D);
                return chunkCount * SPOT_SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<glm::vec2>(BufferNames::SpotLightShadowStaticChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(staticCount, ComputeGroupSize::Buffer32D);

                return chunkCount * SPOT_SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && !pool->GetStorage().GetStaticEntities().empty();
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<VisibleModelData>(BufferNames::PointLightShadowModelVisibleData,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool ? static_cast<uint32_t>(pool->Size()) * POINT_SHADOW_MULTIPLIER : 0;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<ModelComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<glm::vec2>(BufferNames::PointLightShadowMortonChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(static_cast<uint32_t>(pool->Size()), ComputeGroupSize::Buffer32D);
                return chunkCount * POINT_SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && pool->Size() > 0;
            },
            ComponentMemoryType::GpuOnly);

        RegisterGenericBuffer<glm::vec2>(BufferNames::PointLightShadowStaticChunkVisibleIndex,
            [this]() -> uint32_t {
                auto pool = _registry->GetPool<TransformComponent>();
                if (!pool) return 0;

                uint32_t staticCount = static_cast<uint32_t>(pool->GetStorage().GetStaticEntities().size());
                uint32_t chunkCount = ComputeGroupSize::CalculateDispatchCount(staticCount, ComputeGroupSize::Buffer32D);

                return chunkCount * POINT_SHADOW_MULTIPLIER;
            },
            [this]() -> bool {
                auto pool = _registry->GetPool<TransformComponent>();
                return pool && !pool->GetStorage().GetStaticEntities().empty();
            },
            ComponentMemoryType::GpuOnly);
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

                CpuProfileScope profile(ServiceLocator::Get<ICpuProfiler>(), _currentFrameIndex, groupName, profilerName);

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

    void Scene::Update(float deltaTime, uint32_t frameIndex)
    {
        _currentFrameIndex = frameIndex;
        _currentDeltaTime = deltaTime;

        auto modelSnapshot = ServiceLocator::Get<ModelManager>()->GetSnapshotAndVersion();
        auto animSnapshot = ServiceLocator::Get<AnimationManager>()->GetSnapshotAndVersion();
        auto materialSnapshot = ServiceLocator::Get<MaterialManager>()->GetSnapshotAndVersion();
		auto audioSnapshot = ServiceLocator::Get<AudioManager>()->GetSnapshotAndVersion();

        _systemContext.deltaTime = deltaTime;
        _systemContext.frameIndex = frameIndex;

        _systemContext.modelManagerVersion = modelSnapshot.version;
        _systemContext.materialManagerVersion = materialSnapshot.version;
        _systemContext.animationManagerVersion = animSnapshot.version;
        _systemContext.audioManagerVersion = audioSnapshot.version;

        _systemContext.modelSnapshots = modelSnapshot.snapshots;
        _systemContext.materialSnapshots = materialSnapshot.snapshots;
        _systemContext.animationSnapshots = animSnapshot.snapshots;
        _systemContext.soundSnapshots = audioSnapshot.snapshots;

        _systemContext.materialRenderTypes.clear();
        _systemContext.materialRenderTypes.resize(materialSnapshot.snapshots.size());

        std::transform(materialSnapshot.snapshots.begin(), materialSnapshot.snapshots.end(), _systemContext.materialRenderTypes.begin(),
            [](const auto& snapshot) -> MaterialRenderType {
                if (!snapshot.resource) 
                    return MaterialRenderType::Opaque1Sided;

                bool isTrans = snapshot.resource->isTransparent;
                bool isAlpha = snapshot.resource->isAlphaTested;
                bool isDouble = snapshot.resource->doubleSided;

                if (isTrans) {
                    if (isAlpha) 
                        return isDouble ? MaterialRenderType::AlphaTestedTransparent2Sided : MaterialRenderType::AlphaTestedTransparent1Sided;
                    else
                        return isDouble ? MaterialRenderType::Transparent2Sided : MaterialRenderType::Transparent1Sided;
                }
                else {
                    if (isAlpha) 
                        return isDouble ? MaterialRenderType::AlphaTestedOpaque2Sided : MaterialRenderType::AlphaTestedOpaque1Sided;
                    else
                        return isDouble ? MaterialRenderType::Opaque2Sided : MaterialRenderType::Opaque1Sided;
                }
            });

        auto screenWidth = ServiceLocator::Get<FrameContext>()->screenWidth;
        auto screenHeight = ServiceLocator::Get<FrameContext>()->screenHeight;

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

        ServiceLocator::Get<tf::Executor>()->run(_updateTaskflow).wait();
    }

    void Scene::UpdateGPU(uint32_t frameIndex)
    {
        _currentFrameIndex = frameIndex;
        _componentBufferManager->Update(frameIndex);

        ServiceLocator::Get<tf::Executor>()->run(_gpuTaskflow).wait();
    }

    void Scene::Finish()
    {
        ServiceLocator::Get<tf::Executor>()->run(_finishTaskflow).wait();
    }
}