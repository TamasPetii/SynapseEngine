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
#include <glm/glm.hpp>
#include "Engine/Image/Source/Procedural/ProceduralImageSource.h"

namespace Syn
{
    class SYN_API BrdfLutImageSource : public ProceduralImageSource
    {
    public:
        BrdfLutImageSource(uint32_t resolution = 512, uint32_t sampleCount = 1024);
        ~BrdfLutImageSource() override = default;

        std::optional<RawImage> Produce() override;
    private:
        float RadicalInverse_VdC(uint32_t bits);
        glm::vec2 Hammersley(uint32_t i, uint32_t N);
        glm::vec3 ImportanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness);
        float GeometrySchlickGGX(float NdotV, float roughness);
        float GeometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness);
        glm::vec2 IntegrateBRDF(float NdotV, float roughness);
    private:
        uint32_t _resolution;
        uint32_t _sampleCount;
    };
}