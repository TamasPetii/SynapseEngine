#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SystemPhaseNames
    {
        static constexpr const char* Finish = "Finish";
        static constexpr const char* FinishResetState = "Finish Reset State";
        static constexpr const char* Update = "Update";
        static constexpr const char* UploadSparseMap = "Upload Sparse Map";
        static constexpr const char* UploadGPU = "Upload GPU";
        

        static constexpr const char* Stream = "Stream";
        static constexpr const char* Dynamic = "Dynamic";
        static constexpr const char* DynamicFiltered = "DynamicFiltered";
        static constexpr const char* StaticDirty = "StaticDirty";
        static constexpr const char* Static = "Static";
    };

}