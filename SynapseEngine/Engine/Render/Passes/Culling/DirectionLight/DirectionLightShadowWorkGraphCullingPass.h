#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API DirectionLightShadowWorkGraphCullingPass : public ComputePass {
    public:
        ~DirectionLightShadowWorkGraphCullingPass();

        std::string GetName() const override { return "DirectionLightShadowWorkGraphCullingPass"; }
        std::string GetGroup() const override { return PassGroupNames::DirectionLightShadowCullingPasses; }

        void Initialize() override;
        void Execute(const RenderContext& context) override;

    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;

    private:
        uint32_t _dynamicModelCount = 0;
        uint32_t _staticChunkCount = 0;
        uint32_t _mortonChunkCount = 0;
        uint32_t _activeDirectionLightShadowCount = 0;

        uint32_t _dynamicModelRootIndex = 0;
        uint32_t _staticChunkRootIndex = 0;
        uint32_t _mortonChunkRootIndex = 0;

        std::shared_ptr<Vk::Buffer> _scratchBuffer;
        VkPipeline _graphPipeline = VK_NULL_HANDLE;
    };
}