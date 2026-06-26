#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

namespace Syn {
    class SYN_API GeometryWorkGraphCullingPass : public ComputePass {
    public:
        ~GeometryWorkGraphCullingPass();

        std::string GetName() const override { return "GeometryWorkGraphCullingPass"; }
        std::string GetGroup() const override { return PassGroupNames::GeometryCullingPasses; }

        void Initialize() override;
        void Execute(const RenderContext& context) override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PushConstants(const RenderContext& context) override;
        void BindDescriptors(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        // Variables to track the sizes of our 3 Root Node dispatches
        uint32_t _dynamicModelCount = 0;
        uint32_t _staticChunkCount = 0;
        uint32_t _mortonChunkCount = 0;

        // Node indices mapped after pipeline creation (Depends on your specific Vulkan AMDX wrapper)
        uint32_t _dynamicModelRootIndex = 0;
        uint32_t _staticChunkRootIndex = 0;
        uint32_t _mortonChunkRootIndex = 0;

        std::shared_ptr<Vk::Buffer> _scratchBuffer;
        VkPipeline _graphPipeline = VK_NULL_HANDLE;
    };
}