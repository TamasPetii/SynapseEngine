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

#include "Renderer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk//Command/CommandBuffer.h"
#include "Engine/Vk/Synchronization/Fence.h"
#include "Engine/Vk/Synchronization/BinarySemaphore.h"
#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    Renderer::Renderer(uint32_t framesInFlight)
        : _framesInFlight(framesInFlight)
    {
        auto vkContext = ServiceLocator::Get<Vk::Context>();
        auto device = vkContext->GetDevice();

        _graphicsQueue = device->GetGraphicsQueue();

        _commandPool = std::make_unique<Vk::CommandPool>(
            _graphicsQueue,
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        );

        _commandBuffers.reserve(_framesInFlight);
        _imageAvailableSemaphores.reserve(_framesInFlight);
        _renderFinishedSemaphores.reserve(_framesInFlight);
        _inFlightFences.reserve(_framesInFlight);

        for (uint32_t i = 0; i < _framesInFlight; i++) {
            _imageAvailableSemaphores.push_back(std::make_unique<Vk::BinarySemaphore>());
            _renderFinishedSemaphores.push_back(std::make_unique<Vk::BinarySemaphore>());
            _inFlightFences.push_back(std::make_unique<Vk::Fence>(true));
            _presentFences.push_back(std::make_unique<Vk::Fence>(true));
            _commandBuffers.push_back(_commandPool->AllocateBuffer());
        }
    }

    Renderer::~Renderer() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
        device->WaitIdle();
    }

    void Renderer::WaitForFrame(uint32_t frameIndex) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();

        VkFence fences[] = { _inFlightFences[frameIndex]->Handle(), _presentFences[frameIndex]->Handle() };
        VkResult result = vkWaitForFences(device, 2, fences, VK_TRUE, UINT64_MAX);

        if (result != VK_SUCCESS) {
            Error("Renderer::WaitForFrame: Error while waiting for fences! FrameIndex: {}, Result: {}", frameIndex, (int)result);
            SYN_VK_ASSERT_MSG(result, "Error while waiting for fences!");
        }
    }

    Vk::CommandBuffer* Renderer::BeginFrame(uint32_t frameIndex) {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();
        auto swapChain = ServiceLocator::Get<Vk::Context>()->GetSwapChain();

        VkFence fences[] = { _inFlightFences[frameIndex]->Handle(), _presentFences[frameIndex]->Handle() };
        vkResetFences(device, 2, fences);

        uint32_t imageIndex = swapChain->AcquireNextImage(_imageAvailableSemaphores[frameIndex]->Handle());

        if (imageIndex == static_cast<uint32_t>(-1)) {
            return nullptr;
        }

        _imageIndex = imageIndex;

        auto cmd = _commandBuffers[frameIndex].get();
        cmd->Reset();
        cmd->Begin();

        return cmd;
    }

    void Renderer::EndFrame(uint32_t frameIndex) {
        auto cmd = _commandBuffers[frameIndex].get();
        cmd->End();

        VkSemaphoreSubmitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
        waitInfo.semaphore = _imageAvailableSemaphores[frameIndex]->Handle();
        waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSemaphoreSubmitInfo signalInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
        signalInfo.semaphore = _renderFinishedSemaphores[frameIndex]->Handle();
        signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkCommandBufferSubmitInfo cmdInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        cmdInfo.commandBuffer = cmd->Handle();

        VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        submitInfo.waitSemaphoreInfoCount = 1;
        submitInfo.pWaitSemaphoreInfos = &waitInfo;
        submitInfo.signalSemaphoreInfoCount = 1;
        submitInfo.pSignalSemaphoreInfos = &signalInfo;
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdInfo;

        _graphicsQueue->Submit(&submitInfo, _inFlightFences[frameIndex]->Handle());

        auto swapChain = ServiceLocator::Get<Vk::Context>()->GetSwapChain();
        swapChain->Present(_imageIndex, _renderFinishedSemaphores[frameIndex]->Handle(), _presentFences[frameIndex]->Handle());
    }
}