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