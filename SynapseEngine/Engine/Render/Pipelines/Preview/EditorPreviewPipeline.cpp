// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "EditorPreviewPipeline.h"
#include "Engine/Render/Passes/Preview/PreviewPreTransitionPass.h"
#include "Engine/Render/Passes/Preview/ModelPreviewPass.h"
#include "Engine/Render/Passes/Preview/AnimationPreviewPass.h"
#include "Engine/Render/Passes/Preview/MaterialPreviewPass.h"
#include "Engine/Render/Passes/Preview/AudioPreviewPass.h"
#include "Engine/Render/Passes/Preview/MaterialPreviewBloomPass.h"
#include "Engine/Render/Passes/Preview/PreviewPostTransitionPass.h"

namespace Syn {
    EditorPreviewPipeline::EditorPreviewPipeline()
        : RenderPipeline("EditorPreviewPhase", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<PreviewPreTransitionPass>());
        AddPass(std::make_unique<MaterialPreviewPass>());
        AddPass(std::make_unique<ModelPreviewPass>());
        AddPass(std::make_unique<AudioPreviewPass>());
        AddPass(std::make_unique<AnimationPreviewPass>());
        AddPass(std::make_unique<MaterialPreviewBloomPass>());
        AddPass(std::make_unique<PreviewPostTransitionPass>());
    }

    bool EditorPreviewPipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}