#include "TestMEshPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/Data/StaticMesh.h"
#include "Engine/Manager/ModelManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Logger/SynLog.h"
#include "Engine/Vk/Image/ImageFactory.h"

#include <Windows.h>

namespace Syn 
{
    struct RenderPushConstants {
        VkDeviceAddress positionsAddr;
        VkDeviceAddress attributesAddr;
        VkDeviceAddress indicesAddr;
        VkDeviceAddress nodesAddr;
        glm::mat4 viewProj;
        VkDeviceAddress meshletDescAddr;
        VkDeviceAddress meshletVertexIndicesAddr;
        VkDeviceAddress meshletTriangleIndicesAddr;
        VkDeviceAddress drawDescAddr;
    };

    void TestMeshPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("TestColorProgram", {
                "../Engine/Shaders/TestMesh.mesh",
                "../Engine/Shaders/TestMesh.frag"
            });

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blend = {
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void TestMeshPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto image = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = swapChain->GetExtent();

        _graphicsState.renderArea = extent;

        static std::unique_ptr<Vk::Image> depthImage = nullptr;

        if (!depthImage || depthImage->GetExtent().width != extent.width || depthImage->GetExtent().height != extent.height) {
            depthImage = Vk::ImageFactory::CreateAttachment(
                extent.width,
                extent.height,
                VK_FORMAT_D32_SFLOAT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }

        Vk::AttachmentConfig colorConfig = {
            .imageView = image->GetView("_default"),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{{{0.1f, 0.1f, 0.1f, 1.0f}}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        Vk::AttachmentConfig depthConfig = {
            .imageView = depthImage->GetView("_default"),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{.depthStencil = {1.0f, 0}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        _colorAttachments = { Vk::RenderUtils::CreateAttachment(colorConfig) };
        _depthAttachment = Vk::RenderUtils::CreateAttachment(depthConfig);

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = _depthAttachment.has_value() ? &_depthAttachment.value() : nullptr,
            .stencilAttachment = _stencilAttachment.has_value() ? &_stencilAttachment.value() : nullptr,
            .layerCount = 1
        };

        _imageTransitions.clear();

        _imageTransitions.push_back({
            .image = image,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });

        _imageTransitions.push_back({
            .image = depthImage.get(),
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });
    }

    void TestMeshPass::PushConstants(const RenderContext & context) {
        auto modelManager = ServiceLocator::GetModelManager();
        auto sponza = modelManager->GetResource("C:/Users/User/Desktop/Models/Sponza-master/sponza.obj");

        if (!sponza || sponza->hardwareBuffers.indirectMeshletBuffer == nullptr)
            return;

        static glm::vec3 camPos = glm::vec3(-12.0f, 2.5f, 0.0f);
        static float yaw = 0.0f;
        static float pitch = 0.0f;

        float moveSpeed = 0.05f;
        float rotSpeed = 0.002f;

        if (GetAsyncKeyState(VK_LEFT) & 0x8000)  yaw -= rotSpeed;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) yaw += rotSpeed;
        if (GetAsyncKeyState(VK_UP) & 0x8000)    pitch += rotSpeed;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)  pitch -= rotSpeed;

        pitch = glm::clamp(pitch, -1.5f, 1.5f);

        glm::vec3 front;
        front.x = cos(yaw) * cos(pitch);
        front.y = sin(pitch);
        front.z = sin(yaw) * cos(pitch);
        front = glm::normalize(front);

        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        if (GetAsyncKeyState('W') & 0x8000) camPos += front * moveSpeed;
        if (GetAsyncKeyState('S') & 0x8000) camPos -= front * moveSpeed;
        if (GetAsyncKeyState('A') & 0x8000) camPos -= right * moveSpeed;
        if (GetAsyncKeyState('D') & 0x8000) camPos += right * moveSpeed;
        if (GetAsyncKeyState('Q') & 0x8000) camPos -= up * moveSpeed;
        if (GetAsyncKeyState('E') & 0x8000) camPos += up * moveSpeed;

        glm::vec3 camTarget = camPos + front;
        glm::mat4 view = glm::lookAt(camPos, camTarget, up);
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 5000.0f);
        proj[1][1] *= -1.0f;

        RenderPushConstants pc{};
        pc.positionsAddr = sponza->hardwareBuffers.vertexPositions->GetDeviceAddress();
        pc.attributesAddr = sponza->hardwareBuffers.vertexAttributes->GetDeviceAddress();
        pc.indicesAddr = sponza->hardwareBuffers.indices->GetDeviceAddress();
        pc.nodesAddr = sponza->hardwareBuffers.nodeTransforms->GetDeviceAddress();
        pc.viewProj = proj * view;

        pc.meshletDescAddr = sponza->hardwareBuffers.meshletDescriptors->GetDeviceAddress();
        pc.meshletVertexIndicesAddr = sponza->hardwareBuffers.meshletVertexIndices->GetDeviceAddress();
        pc.meshletTriangleIndicesAddr = sponza->hardwareBuffers.meshletTriangleIndices->GetDeviceAddress();
        pc.drawDescAddr = sponza->hardwareBuffers.meshletDrawDescriptors->GetDeviceAddress();

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL_GRAPHICS,
            0,
            sizeof(RenderPushConstants),
            &pc
        );
    }

    void TestMeshPass::Draw(const RenderContext& context)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto sponza = modelManager->GetResource("C:/Users/User/Desktop/Models/Sponza-master/sponza.obj");

        if (!sponza || sponza->hardwareBuffers.indirectMeshletBuffer == nullptr)
            return;

        vkCmdDrawMeshTasksIndirectEXT(
            context.cmd,
            sponza->hardwareBuffers.indirectMeshletBuffer->Handle(),
            0,
            static_cast<uint32_t>(sponza->gpuData.meshletData.drawDescriptors.size()),
            sizeof(VkDrawMeshTasksIndirectCommandEXT)
        );
    }
}