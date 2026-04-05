#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API WorkGraphCullingPass : public ComputePass {
    public:
        ~WorkGraphCullingPass();

        std::string GetName() const override { return "WorkGraphCullingPass"; }
        void Initialize() override;
        void Execute(const RenderContext& context) override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _totalModelsToTest = 0;
        uint32_t _rootNodeIndex = 0;
        std::shared_ptr<Vk::Buffer> _scratchBuffer;
        VkPipeline _graphPipeline = VK_NULL_HANDLE;
    };
}