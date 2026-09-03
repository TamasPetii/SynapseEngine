#include "DirectionLightCpuShadowPipeline.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowInitPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalOpaquePass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletOpaquePass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletOpaqueAlphaTestedPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTransparentTransitionPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowTraditionalTransparentPass.h"
#include "Engine/Render/Passes/Shadow/DirectionLight/DirectionLightShadowMeshletTransparentPass.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    DirectionLightCpuShadowPipeline::DirectionLightCpuShadowPipeline()
        : RenderPipeline("DirectionLightCpuShadowPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightShadowInitPass>());

        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaquePass>(MaterialRenderType::Opaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletOpaqueAlphaTestedPass>(MaterialRenderType::AlphaTestedOpaque2Sided));

        AddPass(std::make_unique<DirectionLightShadowTransparentTransitionPass>());
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::Transparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowTraditionalTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent1Sided));
        AddPass(std::make_unique<DirectionLightShadowMeshletTransparentPass>(MaterialRenderType::AlphaTestedTransparent2Sided));
    }

    bool DirectionLightCpuShadowPipeline::ShouldExecute(const RenderContext& context) const {
        if (!context.scene) return false;
        return context.scene->GetSettings()->culling.directionLightShadowCullingDevice == CullingDeviceType::CPU;
    }
}