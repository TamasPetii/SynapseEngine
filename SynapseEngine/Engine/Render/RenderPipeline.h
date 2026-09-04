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
#include <vector>
#include <memory>
#include <string>

#include "IRenderPass.h"

namespace Syn
{
    class SYN_API RenderPipeline : public IRenderPass
    {
    public:
        RenderPipeline(const std::string& name = "RenderPipeline", const std::string& group = PassGroupNames::UndefinedPasses)
            : _name(name), _groupName(group) {}

        RenderPipeline(const RenderPipeline&) = delete;
        RenderPipeline& operator=(const RenderPipeline&) = delete;

        void AddPass(std::unique_ptr<IRenderPass> pass);

        virtual void Initialize() override;
        virtual void Execute(const RenderContext& context) override;
        virtual bool ShouldExecute(const RenderContext& context) const override { return !_passes.empty(); }
        virtual std::string GetName() const override { return _name; }
        virtual std::string GetGroup() const override { return _groupName; }
        virtual bool CanExecuteWhileCompiling() const override { return true; }
    protected:
        std::vector<std::unique_ptr<IRenderPass>> _passes;
        std::string _name;
        std::string _groupName;
    };
}