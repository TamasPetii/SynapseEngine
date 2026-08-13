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
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Engine/Image/SamplerNames.h"

namespace Syn
{
    struct TexturePayload {
        std::string path = "";
        std::string formatHint = "";
        std::vector<uint8_t> embeddedData;
        bool isUncompressed = false;
        uint32_t width = 0;
        uint32_t height = 0;

        std::string wrapModeU = SamplerWrapModeNames::Repeat;
        std::string wrapModeV = SamplerWrapModeNames::Repeat;
        std::string wrapModeW = SamplerWrapModeNames::Repeat;

        bool IsEmbedded() const { return !embeddedData.empty(); }
    };

    struct SYN_API MaterialInfo {
        std::string name = "";
        TexturePayload albedo;
        TexturePayload normal;
        TexturePayload metalness;
        TexturePayload roughness;
        TexturePayload metallicRoughness;
        TexturePayload emissive;
        TexturePayload ambientOcclusion;
        TexturePayload opacity;
        TexturePayload clearcoat;
        TexturePayload clearcoatRoughness;
        TexturePayload clearcoatNormal;
        TexturePayload specular;
        TexturePayload specularColor;

        glm::vec4 color = glm::vec4(1.0f);
        glm::vec3 emissiveFactor = glm::vec3(0.0f);
        float emissiveIntensity = 1.0f;
        glm::vec2 uvScale = glm::vec2(1.0f);
        float metallicFactor = 0.0f;
        float roughnessFactor = 1.0f;
        float aoStrength = 1.0f;
        bool doubleSided = false;
        bool isTransparent = false;
        bool isAlphaTested = false;

        float clearcoatFactor = 0.0f;
        float clearcoatRoughnessFactor = 0.0f;
        float ior = 1.5f;
        float specularFactor = 1.0f;
        glm::vec3 specularColorFactor = glm::vec3(1.0f);
    };
}