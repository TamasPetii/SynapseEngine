#pragma once
#include "ISystem.h"
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    template <typename TComponent>
    class ComponentSystem : public ISystem
    {
    public:
        virtual std::vector<TypeID> GetReadDependencies() const override { return {}; }
        virtual std::vector<TypeID> GetWriteDependencies() const override { return { TypeInfo<TComponent>::ID }; }

        virtual void OnFinish(std::shared_ptr<Registry> registry, tf::Subflow& subflow) override
        {
            auto pool = registry->GetPool<TComponent>();
            if (!pool) return;

            auto processFinish = [&pool](EntityID entity) {
                if (pool->template IsBitSet<CHANGED_BIT>(entity))
                    pool->template ResetBit<CHANGED_BIT>(entity);

                if (pool->template IsBitSet<UPDATE_BIT>(entity))
                    pool->template ResetBit<UPDATE_BIT>(entity);
                };

            ParallelForEach(pool, subflow, processFinish);
        }

    protected:
        template <typename TPool, typename Func>
        void ParallelForEach(TPool* pool, tf::Subflow& subflow, Func&& func)
        {
            auto streamSpan = pool->GetStorage().GetStreamEntities();
            if (!streamSpan.empty())
                subflow.for_each(streamSpan.begin(), streamSpan.end(), func);

            auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
            if (!dynamicSpan.empty())
            {
                if (true) //Todo: Check pool flag! Skip if no update!
                {
                    subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), [pool, func](EntityID entity) {
                        if (pool->template IsBitSet<UPDATE_BIT>(entity))
                            func(entity);
                        });
                }
            }

            auto staticDirtySpan = pool->GetStorage().GetDirtyStatics();
            if (!staticDirtySpan.empty())
                subflow.for_each(staticDirtySpan.begin(), staticDirtySpan.end(), func);
        }
    };
}