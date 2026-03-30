#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* TraditionalVert = "../Engine/Shaders/Traditional.vert";
        static constexpr const char* TraditionalFrag = "../Engine/Shaders/Traditional.frag";
        static constexpr const char* TraditionalTransparentFrag = "../Engine/Shaders/TraditionalTransparent.frag";
        static constexpr const char* TraditionalTransparentPickingFrag = "../Engine/Shaders/TraditionalTransparentPicking.frag";
        
        static constexpr const char* MeshletOldMesh = "../Engine/Shaders/MeshletOld.mesh";
        static constexpr const char* MeshletMesh = "../Engine/Shaders/Meshlet.mesh";
        static constexpr const char* MeshletTask = "../Engine/Shaders/Meshlet.task";
        static constexpr const char* MeshletFrag = "../Engine/Shaders/Meshlet.frag";
        static constexpr const char* MeshletTransparentFrag = "../Engine/Shaders/MeshletTransparent.frag";
        static constexpr const char* MeshletTransparentPickingFrag = "../Engine/Shaders/MeshletTransparentPicking.frag";
        
        static constexpr const char* MeshCulling = "../Engine/Shaders/MeshCulling.comp";
        static constexpr const char* ModelCulling = "../Engine/Shaders/ModelCulling.comp";

        static constexpr const char* HizLinearizeDepth = "../Engine/Shaders/HizLinearizeDepth.comp";
        static constexpr const char* HizDownsample = "../Engine/Shaders/HizDownsample.comp";

        static constexpr const char* FullscreenVert = "../Engine/Shaders/Fullscreen.vert";
        static constexpr const char* CompositeFrag = "../Engine/Shaders/Composite.frag";
        static constexpr const char* TransparentCompositeFrag = "../Engine/Shaders/TransparentComposite.frag";

        static constexpr const char* BloomPrefilter = "../Engine/Shaders/BloomPrefilter.comp";
        static constexpr const char* BloomUpsample = "../Engine/Shaders/BloomUpsample.comp";
        static constexpr const char* BloomDownsample = "../Engine/Shaders/BloomDownsample.comp";
        static constexpr const char* BloomComposite = "../Engine/Shaders/BloomComposite.comp";

        static constexpr const char* WireframeSetup = "../Engine/Shaders/WireframeSetup.comp";
        static constexpr const char* WireframeVert = "../Engine/Shaders/Wireframe.vert";
        static constexpr const char* WireframeFrag = "../Engine/Shaders/Wireframe.frag";
        static constexpr const char* WireframeMeshletVert = "../Engine/Shaders/WireframeMeshlet.vert";

		static constexpr const char* DeferredEmissiveAoFrag = "../Engine/Shaders/DeferredEmissiveAo.frag";
        static constexpr const char* DeferredPointLightVert = "../Engine/Shaders/DeferredPointLight.vert";
        static constexpr const char* DeferredPointLightFrag = "../Engine/Shaders/DeferredPointLight.frag";
    };
}