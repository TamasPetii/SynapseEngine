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

#pragma once
#include "Engine/Render/IRenderPass.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <memory>
#include <vector>

namespace Syn {
    class MaterialPreviewBloomPass : public IRenderPass {
    public:
        void Initialize() override;
        void Execute(const RenderContext& context) override;
        bool ShouldExecute(const RenderContext& context) const override;

        std::string GetName() const override { return "MaterialPreviewBloomPass"; }
        std::string GetGroup() const override { return PassGroupNames::UtilityPasses; }
    private:
        void DispatchPrefilter(const RenderContext& context, Vk::Image* colorImage, Vk::Image* bloomImage);
        void DispatchDownsample(const RenderContext& context, Vk::Image* bloomImage);
        void DispatchUpsample(const RenderContext& context, Vk::Image* bloomImage);
        void DispatchComposite(const RenderContext& context, Vk::Image* colorImage, Vk::Image* bloomImage);

        uint32_t _prefilterProgramId = UINT32_MAX;
        uint32_t _downsampleProgramId = UINT32_MAX;
        uint32_t _upsampleProgramId = UINT32_MAX;
        uint32_t _compositeProgramId = UINT32_MAX;

        Vk::ShaderProgram* _prefilterProgram;
        Vk::ShaderProgram* _downsampleProgram;
        Vk::ShaderProgram* _upsampleProgram;
        Vk::ShaderProgram* _compositeProgram;
    };
}