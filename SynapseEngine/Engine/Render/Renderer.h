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
#include "Engine/SynApi.h"
#include <vector>
#include <memory>

namespace Syn::Vk {
    class CommandPool;
    class CommandBuffer;
    class Fence;
    class BinarySemaphore;
    class ThreadSafeQueue;
}

namespace Syn {
    class SYN_API Renderer {
    public:
        explicit Renderer(uint32_t framesInFlight);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        void WaitForFrame(uint32_t frameIndex);
        Vk::CommandBuffer* BeginFrame(uint32_t frameIndex);
        void EndFrame(uint32_t frameIndex);

		uint32_t GetCurrentImageIndex() const { return _imageIndex; }
    private:
        uint32_t _framesInFlight;
        uint32_t _imageIndex = 0;

        Vk::ThreadSafeQueue* _graphicsQueue = nullptr;
        std::unique_ptr<Vk::CommandPool> _commandPool;
        std::vector<std::unique_ptr<Vk::CommandBuffer>> _commandBuffers;
        std::vector<std::unique_ptr<Vk::BinarySemaphore>> _imageAvailableSemaphores;
        std::vector<std::unique_ptr<Vk::BinarySemaphore>> _renderFinishedSemaphores;
        std::vector<std::unique_ptr<Vk::Fence>> _inFlightFences;
        std::vector<std::unique_ptr<Vk::Fence>> _presentFences;
    };
}