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
#include "IVideoProcessorPipeline.h"

namespace Syn
{
    class SYN_API VideoProcessorPipeline : public IVideoProcessorPipeline
    {
    public:
        VideoProcessorPipeline() = default;

        VideoProcessorPipeline(const VideoProcessorPipeline&) = delete;
        VideoProcessorPipeline& operator=(const VideoProcessorPipeline&) = delete;

        void AddProcessor(std::unique_ptr<IVideoProcessor> processor) override;
        void Run(CookedVideoPacket& cookedPacket) override;
    private:
        std::vector<std::unique_ptr<IVideoProcessor>> _processors;
    };
}