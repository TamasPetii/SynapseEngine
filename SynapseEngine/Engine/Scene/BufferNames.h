#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API BufferNames
    {
        static constexpr const char* TransformSparseMap = "TransformSparseMap";
        static constexpr const char* TransformData = "TransformData";

        static constexpr const char* CameraSparseMap = "CameraSparseMap";
        static constexpr const char* CameraData = "CameraData";

        static constexpr const char* ModelSparseMap = "ModelSparseMap";
        static constexpr const char* ModelData = "ModelData";
        static constexpr const char* ModelVisibleData = "ModelVisibleData";

        static constexpr const char* AnimationSparseMap = "AnimationSparseMap";
        static constexpr const char* AnimationData = "AnimationData";

        static constexpr const char* DirectionLightSparseMap = "DirectionLightSparseMap";
        static constexpr const char* DirectionLightData = "DirectionLightData";
        static constexpr const char* DirectionLightVisibleData = "DirectionLightVisibleData";

        static constexpr const char* DirectionLightShadowSparseMap = "DirectionLightShadowSparseMap";
        static constexpr const char* DirectionLightShadowData = "DirectionLightShadowData";
        static constexpr const char* DirectionLightShadowColliderData = "DirectionLightShadowColliderData";

        static constexpr const char* PointLightSparseMap = "PointLightSparseMap";
        static constexpr const char* PointLightData = "PointLightData";
        static constexpr const char* PointLightColliderData = "PointLightColliderData";
		static constexpr const char* PointLightVisibleData = "PointLightVisibleData";

        static constexpr const char* PointLightShadowSparseMap = "PointLightShadowSparseMap";
        static constexpr const char* PointLightShadowData = "PointLightShadowData";

        static constexpr const char* SpotLightSparseMap = "SpotLightSparseMap";
        static constexpr const char* SpotLightData = "SpotLightData";
        static constexpr const char* SpotLightColliderData = "SpotLightColliderData";
        static constexpr const char* SpotLightVisibleData = "SpotLightVisibleData";

        static constexpr const char* SpotLightShadowSparseMap = "SpotLightShadowSparseMap";
        static constexpr const char* SpotLightShadowData = "SpotLightShadowData";
    };
}