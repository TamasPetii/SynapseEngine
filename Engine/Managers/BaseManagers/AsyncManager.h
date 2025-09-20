#pragma once
#include "Engine/EngineApi.h"
#include "Engine/Utils/Hashable.h"

#include <mutex>
#include <string>
#include <future>
#include <vector>
#include <sstream>
#include <unordered_map>



template <Hashable T>
class AsyncManager
{
public:
	AsyncManager() = default;

    AsyncManager(const AsyncManager&) = delete;
    AsyncManager& operator=(const AsyncManager&) = delete;
protected:
	bool HasFuture(const T& id) { return futures.find(id) != futures.end(); }
	std::vector<T> CompleteFinishedFutures();
protected:
	std::mutex asyncMutex;
	std::unordered_map<T, std::future<void>> futures;
    std::stringstream log;
};

template <Hashable T>
inline std::vector<T> AsyncManager<T>::CompleteFinishedFutures()
{
    std::vector<T> completedFutures;
    for (auto it = futures.begin(); it != futures.end();) {
        if (it->second.valid() && it->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            try {
                it->second.get();
                completedFutures.push_back(it->first);
                it = futures.erase(it);
            }
            catch (const std::exception& e) {
                it = futures.erase(it);
            }
        }
        else {
            ++it;
        }
    }

    return completedFutures;
}
