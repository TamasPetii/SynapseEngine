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
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"
#include "Engine/Vk/Buffer/Buffer.h"

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
        void CreateGraphPipeline();

        uint32_t _dynamicModelCount = 0;
        uint32_t _staticChunkCount = 0;
        uint32_t _mortonChunkCount = 0;

        uint32_t _dynamicModelRootIndex = 0;
        uint32_t _staticChunkRootIndex = 0;
        uint32_t _mortonChunkRootIndex = 0;

        std::shared_ptr<Vk::Buffer> _scratchBuffer;
        VkPipeline _graphPipeline = VK_NULL_HANDLE;
    };
}