#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemWithBarrier.h>
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

        JobSystemTaskflow(tf::Executor& executor, JPH::uint inMaxJobs, JPH::uint inMaxBarriers)
            : JPH::JobSystemWithBarrier(inMaxBarriers)
            , mExecutor(executor)
        {
            Init(inMaxJobs);

            mJobStorage.resize(inMaxJobs);
            mFreeJobs.reserve(inMaxJobs);

            for (JPH::uint i = 0; i < inMaxJobs; ++i)
            {
                mFreeJobs.push_back(reinterpret_cast<Job*>(mJobStorage[i].data));
            }
        }

        ~JobSystemTaskflow() override
        {
            mExecutor.wait_for_all();
        }

        int GetMaxConcurrency() const override
        {
            return static_cast<int>(mExecutor.num_workers());
        }

        JPH::JobHandle CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, JPH::uint32 inNumDependencies = 0) override
        {
            Job* job = nullptr;

            {
                std::lock_guard<std::mutex> lock(mJobsMutex);

                if (!mFreeJobs.empty())
                {
                    job = mFreeJobs.back();
                    mFreeJobs.pop_back();
                }
            }

            JPH_ASSERT(job != nullptr);

            if (job == nullptr)
            {
                std::terminate();
            }

            new (job) Job(inName, inColor, this, inJobFunction,inNumDependencies);
            return JPH::JobHandle(job);
        }

    protected:
        void FreeJob(Job* inJob) override
        {
            if (inJob == nullptr)
                return;

            inJob->~Job();

            {
                std::lock_guard<std::mutex> lock(mJobsMutex);
                mFreeJobs.push_back(inJob);
            }
        }

        void QueueJob(Job* inJob) override
        {
            JPH_ASSERT(inJob != nullptr);

            inJob->AddRef();

            mExecutor.silent_async([inJob]()
                {
                    try
                    {
                        inJob->Execute();
                    }
                    catch (...)
                    {
                        JPH_ASSERT(false);
                    }

                    inJob->Release();
                });
        }

        void QueueJobs(Job** inJobs, JPH::uint inNumJobs) override
        {
            for (JPH::uint i = 0; i < inNumJobs; ++i)
            {
                QueueJob(inJobs[i]);
            }
        }

    private:
        tf::Executor& mExecutor;
        std::vector<JobStorage> mJobStorage;
        std::vector<Job*> mFreeJobs;
        std::mutex mJobsMutex;
    };
}