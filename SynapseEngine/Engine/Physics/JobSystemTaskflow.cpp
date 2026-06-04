#include "JobSystemTaskflow.h"
#include <new>

namespace Syn
{
    JobSystemTaskflow::JobSystemTaskflow(tf::Executor& executor, JPH::uint inMaxJobs, JPH::uint inMaxBarriers)
        : JPH::JobSystemWithBarrier(inMaxBarriers)
        , mExecutor(executor)
    {
        mJobStorage.resize(inMaxJobs);
        mFreeJobs.reserve(inMaxJobs);

        for (JPH::uint i = 0; i < inMaxJobs; ++i)
        {
            mFreeJobs.push_back(reinterpret_cast<Job*>(mJobStorage[i].data));
        }
    }

    JobSystemTaskflow::~JobSystemTaskflow()
    {
        mExecutor.wait_for_all();
    }

    int  JobSystemTaskflow::GetMaxConcurrency() const
    {
        return static_cast<int>(mExecutor.num_workers());
    }

    JPH::JobHandle JobSystemTaskflow::CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, JPH::uint32 inNumDependencies)
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

        ::new (job) Job(inName, inColor, this, inJobFunction, inNumDependencies);
        return JPH::JobHandle(job);
    }

    void  JobSystemTaskflow::FreeJob(Job* inJob)
    {
        if (inJob == nullptr)
            return;

        inJob->~Job();

        {
            std::lock_guard<std::mutex> lock(mJobsMutex);
            mFreeJobs.push_back(inJob);
        }
    }

    void JobSystemTaskflow::QueueJob(Job* inJob)
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

    void JobSystemTaskflow::QueueJobs(Job** inJobs, JPH::uint inNumJobs)
    {
        for (JPH::uint i = 0; i < inNumJobs; ++i)
        {
            QueueJob(inJobs[i]);
        }
    }
}