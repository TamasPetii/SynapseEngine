#pragma once
#include "Engine/SynApi.h"
#include "Engine/Utils/WindowedBuffer.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Syn
{
    struct SYN_API ComponentBufferView
    {
        Vk::Buffer* buffer;
        std::span<uint32_t> versions;
    };

    class SYN_API ComponentBufferManager
    {
    public:
        ComponentBufferManager(uint32_t frameCount);

        void RegisterBuffer(const std::string& name, uint32_t elementSize, std::function<uint32_t()> sizeCallback);
        void Update(uint32_t frameIndex);

        ComponentBufferView GetComponentBuffer(const std::string& name, uint32_t frameIndex);
    private:
        struct FrameData
        {
            WindowedBuffer gpuBuffer;
            std::vector<uint32_t> versions;
        };

        struct ComponentBufferState
        {
            std::vector<FrameData> frames;
            std::function<uint32_t()> sizeCallback;
        };

        uint32_t _frameCount;
        std::unordered_map<std::string, ComponentBufferState> _buffers;
    };
}