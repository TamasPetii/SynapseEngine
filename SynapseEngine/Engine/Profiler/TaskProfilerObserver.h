#pragma once
#include <taskflow/taskflow.hpp>
#include <vector>
#include <string>
#include <chrono>

namespace Syn
{
    struct TaskProfileData
    {
        std::string name;
        size_t workerId;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    };

    class TaskProfilerObserver : public tf::ObserverInterface
    {
    public:
        void set_up(size_t num_workers) override final {
            _startTimes.resize(num_workers);
            _workerTasks.resize(num_workers);
            for (auto& tasks : _workerTasks) {
                tasks.reserve(500);
            }
        }

        void on_entry(tf::WorkerView w, tf::TaskView tv) override final {
            _startTimes[w.id()] = std::chrono::high_resolution_clock::now();
        }

        void on_exit(tf::WorkerView w, tf::TaskView tv) override final {
            auto endTime = std::chrono::high_resolution_clock::now();
            size_t id = w.id();

            _workerTasks[id].push_back({
                tv.name(),
                id,
                _startTimes[id],
                endTime
                });
        }

        std::vector<TaskProfileData> ExtractFrameData() {
            std::vector<TaskProfileData> allTasks;
            allTasks.reserve(1000);

            for (auto& tasks : _workerTasks) {
                allTasks.insert(allTasks.end(), tasks.begin(), tasks.end());
                tasks.clear();
            }
            return allTasks;
        }

    private:
        std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> _startTimes;
        std::vector<std::vector<TaskProfileData>> _workerTasks;
    };
}