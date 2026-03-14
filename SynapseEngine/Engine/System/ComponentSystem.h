#pragma once
#include "ISystem.h"
#include <vector>
#include <string>
#include <optional>
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
        std::optional<tf::Task> ForEachStream(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <typename TPool, typename Func>
        std::optional<tf::Task> ForEachDynamic(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <uint32_t FilterBit, typename TPool, typename Func>
        std::optional<tf::Task> ForEachDynamicIf(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <typename TPool, typename Func>
        std::optional<tf::Task> ForEachStaticDirty(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <typename TPool, typename Func>
        std::optional<tf::Task> ForEachStatic(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <typename TPool, typename Func>
        std::vector<tf::Task> ParallelForEach(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);

        template <uint32_t FilterBit, typename TPool, typename Func>
        std::vector<tf::Task> ParallelForEachIf(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func);
    protected:
        std::vector<uint32_t> _gpuDenseVersions;
    };

    template <typename TComponent>
    SYN_INLINE std::vector<TypeID> ComponentSystem<TComponent>::GetReadDependencies() const { return {}; }

    template <typename TComponent>
    SYN_INLINE std::vector<TypeID> ComponentSystem<TComponent>::GetWriteDependencies() const { return {}; }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto pool = scene->GetRegistry()->template GetPool<TComponent>();
        if (pool) UpdateComponents(scene, frameIndex, deltaTime, subflow);
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto pool = scene->GetRegistry()->template GetPool<TComponent>();
        if (!pool) return;

        if (pool->template IsStateBitSet<INDEX_CHANGED_BIT>()) pool->IncrementMappingVersion();
        if (pool->template IsStateBitSet<CHANGED_BIT>()) pool->IncrementChangeVersion();

        std::string sparseName = GetSparseBufferName();
        if (!sparseName.empty())
        {
            auto componentBufferManager = scene->GetComponentBufferManager();
            this->EmplaceTask(subflow, SystemPhaseNames::UploadSparseMap, [pool, componentBufferManager, frameIndex, sparseName]() {
                auto mappingBufferView = componentBufferManager->GetComponentBuffer(sparseName, frameIndex);
                if (mappingBufferView.buffer && mappingBufferView.versions[0] != pool->GetMappingVersion())
                {
                    //Todo: Paged sparse map??
                    auto sparseIndices = pool->GetMapping().GetSparseIndices();
                    void* dst = mappingBufferView.buffer->Map();
                    std::memcpy(dst, sparseIndices.data(), sparseIndices.size() * sizeof(DenseIndex));
                    mappingBufferView.versions[0] = pool->GetMappingVersion();
                }
                });
        }

        bool uploadDynamic = ShouldUploadDenseData(pool, frameIndex);
        bool hasStream = !pool->GetStorage().GetStreamEntities().empty();

        if (hasStream || uploadDynamic) UploadComponents(scene, frameIndex, subflow, uploadDynamic);
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        auto pool = scene->GetRegistry()->template GetPool<TComponent>();
        if (!pool) return;

        bool hasChanged = pool->template IsStateBitSet<CHANGED_BIT>();
        bool hasUpdate = pool->template IsStateBitSet<UPDATE_BIT>();
        bool hasIndex = pool->template IsStateBitSet<INDEX_CHANGED_BIT>();

        if (!hasChanged && !hasUpdate && !hasIndex) return;

        ParallelForEach(pool, subflow, SystemPhaseNames::Finish, [pool](EntityID entity) {
            if (pool->template IsBitSet<CHANGED_BIT>(entity)) pool->template ResetBit<CHANGED_BIT>(entity);
            if (pool->template IsBitSet<UPDATE_BIT>(entity)) pool->template ResetBit<UPDATE_BIT>(entity);
            if (pool->template IsBitSet<INDEX_CHANGED_BIT>(entity)) pool->template ResetBit<INDEX_CHANGED_BIT>(entity);
            });

        this->EmplaceTask(subflow, SystemPhaseNames::FinishResetState, [pool]() {
            pool->ResetAllStateBits();
            pool->ResetStaticDirtyCounter();
            });
    }

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) {}

    template <typename TComponent>
    SYN_INLINE void ComponentSystem<TComponent>::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic) {}

    template <typename TComponent>
    SYN_INLINE std::string ComponentSystem<TComponent>::GetSparseBufferName() const { return ""; }

    template <typename TComponent>
    template <typename TPool>
    SYN_INLINE bool ComponentSystem<TComponent>::ShouldUploadDenseData(TPool* pool, uint32_t frameIndex)
    {
        if (_gpuDenseVersions.size() <= frameIndex) 
            _gpuDenseVersions.resize(frameIndex + 1, 0);

        if (pool->template IsStateBitSet<CHANGED_BIT>())
        {
            _gpuDenseVersions[frameIndex] = pool->GetChangeVersion();
            return true;
        }

        if (_gpuDenseVersions[frameIndex] != pool->GetChangeVersion())
        {
            _gpuDenseVersions[frameIndex] = pool->GetChangeVersion();
            return true;
        }

        return false;
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE std::optional<tf::Task> ComponentSystem<TComponent>::ForEachStream(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        return this->ForEach(pool->GetStorage().GetStreamEntities(), subflow, phaseName + " " + SystemPhaseNames::Stream, std::forward<Func>(func));
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE std::optional<tf::Task> ComponentSystem<TComponent>::ForEachDynamic(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        return this->ForEach(pool->GetStorage().GetDynamicEntities(), subflow, phaseName + " " + SystemPhaseNames::Dynamic, std::forward<Func>(func));
    }

    template <typename TComponent>
    template <uint32_t FilterBit, typename TPool, typename Func>
    SYN_INLINE std::optional<tf::Task> ComponentSystem<TComponent>::ForEachDynamicIf(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (dynamicSpan.empty() || !pool->template IsStateBitSet<FilterBit>()) return std::nullopt;

        return this->ForEach(dynamicSpan, subflow, phaseName + " " + SystemPhaseNames::DynamicFiltered, [pool, func](EntityID entity) {
            if (pool->template IsBitSet<FilterBit>(entity)) func(entity);
            });
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE std::optional<tf::Task> ComponentSystem<TComponent>::ForEachStaticDirty(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        return this->ForEach(pool->GetStorage().GetDirtyStatics(), subflow, phaseName + " " + SystemPhaseNames::StaticDirty, std::forward<Func>(func));
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE std::optional<tf::Task> ComponentSystem<TComponent>::ForEachStatic(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        return this->ForEach(pool->GetStorage().GetStaticEntities(), subflow, phaseName + " " + SystemPhaseNames::Static, std::forward<Func>(func));
    }

    template <typename TComponent>
    template <typename TPool, typename Func>
    SYN_INLINE std::vector<tf::Task> ComponentSystem<TComponent>::ParallelForEach(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        std::vector<tf::Task> tasks;
        if (auto t = ForEachStream(pool, subflow, phaseName, func)) tasks.push_back(*t);
        if (auto t = ForEachDynamic(pool, subflow, phaseName, func)) tasks.push_back(*t);
        if (auto t = ForEachStaticDirty(pool, subflow, phaseName, func)) tasks.push_back(*t);
        return tasks;
    }

    template <typename TComponent>
    template <uint32_t FilterBit, typename TPool, typename Func>
    SYN_INLINE std::vector<tf::Task> ComponentSystem<TComponent>::ParallelForEachIf(TPool* pool, tf::Subflow& subflow, const std::string& phaseName, Func&& func)
    {
        std::vector<tf::Task> tasks;
        if (auto t = ForEachStream(pool, subflow, phaseName, func)) tasks.push_back(*t);
        if (auto t = ForEachDynamicIf<FilterBit>(pool, subflow, phaseName, func)) tasks.push_back(*t);
        if (auto t = ForEachStaticDirty(pool, subflow, phaseName, func)) tasks.push_back(*t);
        return tasks;
    }
}