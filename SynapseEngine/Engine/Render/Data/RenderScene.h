#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "RenderTask.h"
#include "RenderView.h"
#include <vector>
#include <memory>

namespace Syn
{
    struct SYN_API RenderScene
    {
        /*
        uint32_t frameIndex = 0;
        uint32_t cameraIndex = 0;
        VkDeviceAddress cameraBufferAddress = 0;

        //Todo: Direction/Point/Spot -> DeviceAddresses?
        //Fetch ecs -> Render should not know about ecs and registry
        RenderView mainCameraView;
        std::vector<RenderView> shadowViews;
        std::vector<RenderTask> renderTasks;

        // Ez tárolja az összes látható Entity ID-t! (1 elem = 4 byte / uint32_t)
        // Szükséges Flags: VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | HOST_VISIBLE
        std::shared_ptr<Vk::Buffer> globalInstanceBuffer;

        // Ez tárolja a Vulkan Draw Parancsokat! (1 elem = 20 byte / VkDrawIndexedIndirectCommand)
        // Szükséges Flags: VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | HOST_VISIBLE
        std::shared_ptr<Vk::Buffer> globalIndirectCommandBuffer;
        */
    };
}