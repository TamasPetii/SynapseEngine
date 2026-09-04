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

#include "SetupAndInitPipeline.h"
#include "Engine/Render/Passes/Setup/GlobalFrameSetupPass.h"
#include "Engine/Render/Passes/Video/VideoConversionPass.h"
#include "Engine/Render/Passes/Shading/Common/OpaqueInitPass.h"
#include "Engine/Render/Passes/Shading/Common/TransparentInitPass.h"
#include "Engine/Render/Passes/Hiz/HizInitPass.h"

namespace Syn {
    SetupAndInitPipeline::SetupAndInitPipeline()
        : RenderPipeline("SetupAndInitPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<GlobalFrameSetupPass>());
        AddPass(std::make_unique<VideoConversionPass>());
        AddPass(std::make_unique<OpaqueInitPass>());
        AddPass(std::make_unique<TransparentInitPass>());
        AddPass(std::make_unique<HizInitPass>());
    }
}