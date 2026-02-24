#pragma once
#include "Engine/SynApi.h"
#include <atomic>
#include <cstdint>

namespace Syn
{
    struct SYN_API RenderTask
    {
        uint32_t meshIndex;
        uint32_t indexCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;

        std::atomic<uint32_t> instanceCount{ 0 };
        uint32_t* mappedInstanceData = nullptr;

        RenderTask() = default;
        RenderTask(RenderTask&& other) noexcept
            : meshIndex(other.meshIndex),
            indexCount(other.indexCount),
            firstIndex(other.firstIndex),
            vertexOffset(other.vertexOffset),
            mappedInstanceData(other.mappedInstanceData)
        {
            instanceCount.store(other.instanceCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }

        RenderTask& operator=(RenderTask&& other) noexcept
        {
            meshIndex = other.meshIndex;
            indexCount = other.indexCount;
            firstIndex = other.firstIndex;
            vertexOffset = other.vertexOffset;
            mappedInstanceData = other.mappedInstanceData;
            instanceCount.store(other.instanceCount.load(std::memory_order_relaxed), std::memory_order_relaxed);
            return *this;
        }

        RenderTask(const RenderTask&) = delete;
        RenderTask& operator=(const RenderTask&) = delete;
    };
}