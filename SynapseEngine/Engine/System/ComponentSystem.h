#pragma once
#include "ISystem.h"
#include <taskflow/algorithm/for_each.hpp>
#include <vector>
#include <string>
#include "Engine/Scene/BufferNames.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    template <typename TComponent>
    class ComponentSystem : public ISystem
    {
    public:
        virtual std::vector<TypeID> GetReadDependencies() const override;
        virtual std::vector<TypeID> GetWriteDependencies() const override;

        virtual void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        virtual void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        virtual void OnFinish(Scene* scene, tf::Subflow& subflow) override;
    protected:
        virtual void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow);
        virtual void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic);
        virtual std::string GetSparseBufferName() const;
    protected:
        template <typename TPool>
        bool ShouldUploadDenseData(TPool* pool, uint32_t frameIndex);

        template <typename TPool, typename Func>
        void ForEachStream(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <typename TPool, typename Func>
        void ForEachDynamic(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <uint32_t FilterBit, typename TPool, typename Func>
        void ForEachDynamicIf(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <typename TPool, typename Func>
        void ForEachStaticDirty(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <typename TPool, typename Func>
        void ForEachStatic(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <typename TPool, typename Func>
        void ParallelForEach(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <uint32_t FilterBit, typename TPool, typename Func>
        void ParallelForEachIf(TPool* pool, tf::Subflow& subflow, Func&& func);

        template <typename Func>
        tf::Task ExecuteTask(tf::Subflow& subflow, Func&& func);
    protected:
        std::vector<uint32_t> _gpuDenseVersions;
    };

    template <typename TComponent>
    SYN_INLINE std::vector<TypeID> ComponentSystem<TComponent>::GetReadDependencies() const
    {
        return {};
    }

    template <typename TComponent>
    SYN_INLINE std::vector<TypeID> ComponentSystem<TComponent>::GetWriteDependencies() const
    {
        return { TypeInfo<TComponent>::ID };
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<TComponent>();
        if (!pool) return;

        UpdateComponents(scene, frameIndex, deltaTime, subflow);
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();

        auto pool = registry->GetPool<TComponent>();
        if (!pool) return;

        if (pool->template IsStateBitSet<INDEX_CHANGED_BIT>())
        {
            pool->IncrementMappingVersion();
            //Info("[{}] INDEX_CHANGED_BIT detected! Mapping Version bumped to: {}", GetName(), pool->GetMappingVersion());
        }

        if (pool->template IsStateBitSet<CHANGED_BIT>())
        {
            pool->IncrementChangeVersion();
            //Info("[{}] CHANGED_BIT detected! CPU Change Version bumped to: {}", GetName(), pool->GetChangeVersion());
        }

        std::string sparseName = GetSparseBufferName();

        if (!sparseName.empty())
        {
            ExecuteTask(subflow, [this, pool, componentBufferManager, frameIndex, sparseName]() {
                auto mappingBufferView = componentBufferManager->GetComponentBuffer(sparseName, frameIndex);
                if (mappingBufferView.buffer && mappingBufferView.versions[0] != pool->GetMappingVersion())
                {
                    auto sparseIndices = pool->GetMapping().GetSparseIndices();
                    void* dst = mappingBufferView.buffer->Map();
                    std::memcpy(dst, sparseIndices.data(), sparseIndices.size() * sizeof(DenseIndex));
                    mappingBufferView.versions[0] = pool->GetMappingVersion();
                    
                    //Info("[{}] Uploaded Sparse Map to GPU (New Version: {})", GetName(), pool->GetMappingVersion());
                }
            });
        }

        bool uploadDynamic = ShouldUploadDenseData(pool, frameIndex);
        bool hasStream = !pool->GetStorage().GetStreamEntities().empty();

        if (hasStream || uploadDynamic)
        {
            UploadComponents(scene, frameIndex, subflow, uploadDynamic);
        }
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<TComponent>();
        if (!pool) return;

        bool hasChanged = pool->template IsStateBitSet<CHANGED_BIT>();
        bool hasUpdate = pool->template IsStateBitSet<UPDATE_BIT>();
        bool hasIndex = pool->template IsStateBitSet<INDEX_CHANGED_BIT>();

        if (!hasChanged && !hasUpdate && !hasIndex)
            return;

        auto processFinish = [pool](EntityID entity) {
            if (pool->template IsBitSet<CHANGED_BIT>(entity)) pool->template ResetBit<CHANGED_BIT>(entity);
            if (pool->template IsBitSet<UPDATE_BIT>(entity)) pool->template ResetBit<UPDATE_BIT>(entity);
            if (pool->template IsBitSet<INDEX_CHANGED_BIT>(entity)) pool->template ResetBit<INDEX_CHANGED_BIT>(entity);
            };

        ParallelForEach(pool, subflow, processFinish);

        ExecuteTask(subflow, [pool]() {
            pool->ResetAllStateBits();
            pool->ResetStaticDirtyCounter();
            });
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {}

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic)
    {}

    template <typename TComponent>
    SYN_INLINE std::string ComponentSystem<TComponent>::GetSparseBufferName() const
    {
        return "";
    }

    template <typename TComponent>
    template <typename TPool>
    SYN_INLINE bool ComponentSystem<TComponent>::ShouldUploadDenseData(TPool* pool, uint32_t frameIndex)
    {
        if (_gpuDenseVersions.size() <= frameIndex)
            _gpuDenseVersions.resize(frameIndex + 1, 0);

        if (pool->template IsStateBitSet<CHANGED_BIT>())
        {
            _gpuDenseVersions[frameIndex] = pool->GetChangeVersion();
            //Info("[{}] GPU Upload: REQUIRED (Active changes in frame {}). GPU Buffer set to expected Version: {}", GetName(), frameIndex, _gpuDenseVersions[frameIndex]);
            return true;
        }

        if (_gpuDenseVersions[frameIndex] != pool->GetChangeVersion())
        {
            _gpuDenseVersions[frameIndex] = pool->GetChangeVersion();
            //Info("[{}] GPU Upload: REQUIRED (Lagging buffer on frame {}). Catching up to CPU Version: {}", GetName(), frameIndex, pool->GetChangeVersion());
            return true;
        }

        //Info("[{}] GPU Upload: SKIPPED! (Frame {} is up to date at Version: {})", GetName(), frameIndex, pool->GetChangeVersion());
        return false;
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ForEachStream(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        auto streamSpan = pool->GetStorage().GetStreamEntities();
        if (!streamSpan.empty())
        {
            //Info("[{}] ForEachStream: Scheduling {} entities.", GetName(), streamSpan.size());
            subflow.for_each(streamSpan.begin(), streamSpan.end(), func);
        }
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ForEachDynamic(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (!dynamicSpan.empty())
        {
            //Info("[{}] ForEachDynamic: Scheduling {} entities (No Filter).", GetName(), dynamicSpan.size());
            subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), func);
        }
    }

    template <typename TComponent>
    template <uint32_t FilterBit, typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ForEachDynamicIf(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (dynamicSpan.empty()) return;

        if (pool->template IsStateBitSet<FilterBit>())
        {
            //Info("[{}] ForEachDynamicIf: Scheduling {} entities (FilterBit is SET).", GetName(), dynamicSpan.size());
            subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), [pool, func](EntityID entity) {
                if (pool->template IsBitSet<FilterBit>(entity))
                    func(entity);
                });
        }
        else
        {
            //Info("[{}] ForEachDynamicIf: SKIPPED {} entities (FilterBit is NOT SET).", GetName(), dynamicSpan.size());
        }
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ForEachStaticDirty(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        auto staticDirtySpan = pool->GetStorage().GetDirtyStatics();
        if (!staticDirtySpan.empty())
        {
            //Info("[{}] ForEachStaticDirty: Scheduling {} entities.", GetName(), staticDirtySpan.size());
            subflow.for_each(staticDirtySpan.begin(), staticDirtySpan.end(), func);
        }
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ForEachStatic(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        auto staticSpan = pool->GetStorage().GetStaticEntities();
        if (!staticSpan.empty())
        {
            //Info("[{}] ForEachStatic: Scheduling {} entities.", GetName(), staticSpan.size());
            subflow.for_each(staticSpan.begin(), staticSpan.end(), func);
        }
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ParallelForEach(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        ForEachStream(pool, subflow, func);
        ForEachDynamic(pool, subflow, func);
        ForEachStaticDirty(pool, subflow, func);
    }

    template <typename TComponent>
    template <uint32_t FilterBit, typename TPool, typename Func>
    SYN_INLINE void ComponentSystem<TComponent>::ParallelForEachIf(TPool* pool, tf::Subflow& subflow, Func&& func)
    {
        ForEachStream(pool, subflow, func);
        ForEachDynamicIf<FilterBit>(pool, subflow, func);
        ForEachStaticDirty(pool, subflow, func);
    }

    template <typename TComponent>
    template <typename Func>
    SYN_INLINE tf::Task ComponentSystem<TComponent>::ExecuteTask(tf::Subflow& subflow, Func&& func)
    {
        return subflow.emplace(std::forward<Func>(func));
    }
}