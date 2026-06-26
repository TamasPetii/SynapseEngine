#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

#include <chrono>
#include <taskflow/taskflow.hpp>

#include <vector>
#include <mutex>
#include <type_traits>
#include <new>

namespace Syn
{
    class JobSystemTaskflow final : public JPH::JobSystemWithBarrier
    {
    public:
        using Job = JPH::JobSystem::Job;

        struct JobStorage
        {
            alignas(Job) std::byte data[sizeof(Job)];
        };

        JobSystemTaskflow(tf::Executor& executor, JPH::uint inMaxJobs, JPH::uint inMaxBarriers);
        ~JobSystemTaskflow() override;
        int GetMaxConcurrency() const override;
        JPH::JobHandle CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, JPH::uint32 inNumDependencies = 0) override;
    protected:
        void FreeJob(Job* inJob) override;
        void QueueJob(Job* inJob) override;
        void QueueJobs(Job** inJobs, JPH::uint inNumJobs) override;
    private:
        tf::Executor& mExecutor;
        std::vector<JobStorage> mJobStorage;
        std::vector<Job*> mFreeJobs;
        std::mutex mJobsMutex;
    };
}