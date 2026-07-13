#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "HierarchyLevelData.h"
#include <vector>
#include <atomic>
#include <span>

namespace Syn
{
    struct alignas(64) SYN_API LevelQueue
    {
        std::vector<EntityID> entities;
        std::atomic<uint32_t> count{ 0 };

        LevelQueue() = default;

        LevelQueue(LevelQueue&& other) noexcept
            : entities(std::move(other.entities))
        {
            count.store(other.count.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }

        LevelQueue& operator=(LevelQueue&& other) noexcept
        {
            if (this != &other) {
                entities = std::move(other.entities);
                count.store(other.count.load(std::memory_order_relaxed), std::memory_order_relaxed);
            }
            return *this;
        }

        LevelQueue(const LevelQueue&) = delete;
        LevelQueue& operator=(const LevelQueue&) = delete;
    };

    class SYN_API HierarchyWorkQueue
    {
    public:
        HierarchyWorkQueue() = default;

        HierarchyWorkQueue(const HierarchyWorkQueue&) = delete;
        HierarchyWorkQueue& operator=(const HierarchyWorkQueue&) = delete;

        HierarchyWorkQueue(HierarchyWorkQueue&&) = default;
        HierarchyWorkQueue& operator=(HierarchyWorkQueue&&) = default;

        void Initialize(std::span<const HierarchyLevelData> levels);

        SYN_INLINE void Push(uint32_t level, EntityID entity)
        {
            size_t idx = _queues[level].count.fetch_add(1, std::memory_order_relaxed);
            _queues[level].entities[idx] = entity;
        }

        SYN_INLINE std::span<const EntityID> GetQueue(uint32_t level) const
        {
            size_t activeCount = _queues[level].count.load(std::memory_order_relaxed);
            return std::span<const EntityID>(_queues[level].entities.data(), activeCount);
        }

    private:
        std::vector<LevelQueue> _queues;
    };
}