#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Registry/Type/TypeInfo.h"
#include <vector>
#include <string>
#include <optional>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include "Engine/Scene/Scene.h"
#include "SystemPhaseNames.h"

namespace Syn
{
    class SYN_API ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) {};
        virtual void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) {};
        virtual void OnFinish(Scene* scene, tf::Subflow& subflow) {};

        virtual std::vector<TypeID> GetReadDependencies() const { return {}; }
        virtual std::vector<TypeID> GetWriteDependencies() const { return {}; }
        virtual std::string GetName() const = 0;

        SYN_INLINE void SetFramesToUpload(uint32_t frames) { _framesToUpload = frames; }
        SYN_INLINE uint32_t GetFramesToUpload() const { return _framesToUpload; }
        SYN_INLINE bool ShouldForceUpload() const { return _framesToUpload > 0; }
        SYN_INLINE void DecrementFramesToUpload() { if (_framesToUpload > 0) _framesToUpload--; }
    protected:
        template <typename Func>
        tf::Task EmplaceTask(tf::Subflow& subflow, const std::string& taskName, Func&& func);

        template <typename Iterable, typename Func>
        std::optional<tf::Task> ForEach(const Iterable& iterable, tf::Subflow& subflow, const std::string& taskName, Func&& func);

        template <typename Iterator, typename Func>
        std::optional<tf::Task> ForEach(Iterator first, Iterator last, tf::Subflow& subflow, const std::string& taskName, Func&& func);

        template <typename BegType, typename EndType, typename StepType, typename Func>
        std::optional<tf::Task> ForEachIndex(BegType first, EndType last, StepType step, tf::Subflow& subflow, const std::string& taskName, Func&& func);
    protected:
        uint32_t _framesToUpload = 0;
    };

    template <typename Func>
    SYN_INLINE tf::Task ISystem::EmplaceTask(tf::Subflow& subflow, const std::string& taskName, Func&& func)
    {
        tf::Task task = subflow.emplace(std::forward<Func>(func));
        task.name(GetName() + " " + taskName);
        return task;
    }

    template <typename Iterable, typename Func>
    SYN_INLINE std::optional<tf::Task> ISystem::ForEach(const Iterable& iterable, tf::Subflow& subflow, const std::string& taskName, Func&& func)
    {
        if (iterable.empty()) return std::nullopt;
        tf::Task task = subflow.for_each(iterable.begin(), iterable.end(), std::forward<Func>(func));
        task.name(GetName() + " " + taskName);
        return task;
    }

    template <typename Iterator, typename Func>
    SYN_INLINE std::optional<tf::Task> ISystem::ForEach(Iterator first, Iterator last, tf::Subflow& subflow, const std::string& taskName, Func&& func)
    {
        if (first == last) return std::nullopt;
        tf::Task task = subflow.for_each(first, last, std::forward<Func>(func));
        task.name(GetName() + " " + taskName);
        return task;
    }

    template <typename BegType, typename EndType, typename StepType, typename Func>
    SYN_INLINE std::optional<tf::Task> ISystem::ForEachIndex(BegType first, EndType last, StepType step, tf::Subflow& subflow, const std::string& taskName, Func&& func)
    {
        if (first >= last) return std::nullopt;
        tf::Task task = subflow.for_each_index(first, last, step, std::forward<Func>(func));
        task.name(GetName() + " " + taskName);
        return task;
    }
}