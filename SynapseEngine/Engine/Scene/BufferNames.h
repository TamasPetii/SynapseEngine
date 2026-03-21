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
    };
}