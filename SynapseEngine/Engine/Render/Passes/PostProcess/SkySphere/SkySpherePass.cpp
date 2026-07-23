#include "SkySpherePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SkySpherePC.glsl"

    SkySpherePass::SkySpherePass() {}

    bool SkySpherePass::ShouldExecute(const RenderContext& context) const
    {
        auto skyTextureId = context.scene->GetSettings()->environment.skyTextureId;
        bool enabled = context.scene->GetSettings()->environment.enableSky;
        return enabled && skyTextureId != UINT32_MAX && !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void SkySpherePass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgramId = shaderManager->LoadProgramAsync("SkySphereProgram", {
            ShaderNames::SkySphereVert,
            ShaderNames::SkySphereFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
            },
            .blendStates = {
                {
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void SkySpherePass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        std::vector<std::string> targets = {
            RenderTargetNames::Main,
        };

        for (const auto& name : targets)
        {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                    .imageView = group->GetImage(name)->GetView(Vk::ImageViewNames::Default),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                }));
        }

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::OpaqueDepth)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void SkySpherePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t fIdx = context.frameIndex;

        const auto& envSettings = scene->GetSettings()->environment;

        glm::vec3 rotRads = glm::radians(envSettings.skyRotation);
        glm::mat4 rotMat = glm::mat4(1.0f);
        rotMat = glm::rotate(rotMat, rotRads.y, glm::vec3(0.0f, 1.0f, 0.0f));
        rotMat = glm::rotate(rotMat, rotRads.x, glm::vec3(1.0f, 0.0f, 0.0f));
        rotMat = glm::rotate(rotMat, rotRads.z, glm::vec3(0.0f, 0.0f, 1.0f));

        Vk::PushConstant<SkySpherePC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->skyTextureIndex = envSettings.skyTextureId;
        pc->samplerIndex = imageManager->GetSamplerIndex(SamplerNames::SkyboxSampler);
        pc->mappingType = envSettings.skyMode == SkyMode::EquirectangularTexture ? 0 : 1;
        pc->skyIntensity = envSettings.skyIntensity;
        pc->skyRotationMatrix = rotMat;
        pc->skyTint = envSettings.skyTint;
        pc->skyExposureEV = envSettings.skyExposureEV;

        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SkySpherePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();

        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void SkySpherePass::Draw(const RenderContext& context)
    {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}