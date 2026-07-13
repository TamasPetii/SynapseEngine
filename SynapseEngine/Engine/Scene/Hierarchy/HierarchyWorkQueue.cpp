#include "HierarchyWorkQueue.h"

namespace Syn
{
    void HierarchyWorkQueue::Initialize(std::span<const HierarchyLevelData> levels)
    {
        uint32_t maxLevel = static_cast<uint32_t>(levels.size());

        if (_queues.size() < maxLevel) {
            _queues.resize(maxLevel);
        }

        for (uint32_t i = 0; i < maxLevel; ++i) {
            if (_queues[i].entities.size() < levels[i].capacity) {
                _queues[i].entities.resize(levels[i].capacity);
            }

            _queues[i].count.store(0, std::memory_order_relaxed);
        }
    }
}