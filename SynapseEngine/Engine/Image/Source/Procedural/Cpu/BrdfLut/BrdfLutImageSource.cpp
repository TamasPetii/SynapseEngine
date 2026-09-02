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

#include "BrdfLutImageSource.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

namespace Syn
{
    BrdfLutImageSource::BrdfLutImageSource(uint32_t resolution, uint32_t sampleCount)
        : ProceduralImageSource("BrdfLut"), _resolution(resolution), _sampleCount(sampleCount)
    {}

    std::optional<RawImage> BrdfLutImageSource::Produce()
    {
        RawImage image{};
        image.width = _resolution;
        image.height = _resolution;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R32G32_SFLOAT;
        image.isCompressed = false;
        image.autoCache = false;

        size_t imageSizeInBytes = _resolution * _resolution * 2 * sizeof(float);
        image.pixels.resize(imageSizeInBytes);

        float* destPixels = reinterpret_cast<float*>(image.pixels.data());

        for (uint32_t y = 0; y < _resolution; ++y)
        {
            float roughness = static_cast<float>(y) / static_cast<float>(_resolution - 1);
            roughness = std::max(roughness, 0.001f);

            for (uint32_t x = 0; x < _resolution; ++x)
            {
                float NdotV = static_cast<float>(x) / static_cast<float>(_resolution - 1);
                NdotV = std::max(NdotV, 0.001f);

                glm::vec2 integratedBRDF = IntegrateBRDF(NdotV, roughness);

                uint32_t pixelIndex = (y * _resolution + x) * 2;
                destPixels[pixelIndex + 0] = integratedBRDF.x;
                destPixels[pixelIndex + 1] = integratedBRDF.y;
            }
        }

        return image;
    }

    float BrdfLutImageSource::RadicalInverse_VdC(uint32_t bits)
    {
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return static_cast<float>(bits) * 2.3283064365386963e-10f;
    }

    glm::vec2 BrdfLutImageSource::Hammersley(uint32_t i, uint32_t N)
    {
        return glm::vec2(static_cast<float>(i) / static_cast<float>(N), RadicalInverse_VdC(i));
    }

    glm::vec3 BrdfLutImageSource::ImportanceSampleGGX(glm::vec2 Xi, glm::vec3 N, float roughness)
    {
        float a = roughness * roughness;
        float phi = 2.0f * glm::pi<float>() * Xi.x;
        float cosTheta = std::sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
        float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

        glm::vec3 H;
        H.x = std::cos(phi) * sinTheta;
        H.y = std::sin(phi) * sinTheta;
        H.z = cosTheta;

        glm::vec3 up = std::abs(N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 tangent = glm::normalize(glm::cross(up, N));
        glm::vec3 bitangent = glm::cross(N, tangent);

        glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
        return glm::normalize(sampleVec);
    }

    float BrdfLutImageSource::GeometrySchlickGGX(float NdotV, float roughness)
    {
        float a = roughness;
        float k = (a * a) / 2.0f;

        float nom = NdotV;
        float denom = NdotV * (1.0f - k) + k;

        return nom / denom;
    }

    float BrdfLutImageSource::GeometrySmith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness)
    {
        float NdotV = std::max(glm::dot(N, V), 0.0f);
        float NdotL = std::max(glm::dot(N, L), 0.0f);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    glm::vec2 BrdfLutImageSource::IntegrateBRDF(float NdotV, float roughness)
    {
        glm::vec3 V;
        V.x = std::sqrt(1.0f - NdotV * NdotV);
        V.y = 0.0f;
        V.z = NdotV;

        float A = 0.0f;
        float B = 0.0f;

        glm::vec3 N = glm::vec3(0.0f, 0.0f, 1.0f);
        float a2 = roughness * roughness;

        for (uint32_t i = 0u; i < _sampleCount; ++i)
        {
            glm::vec2 Xi = Hammersley(i, _sampleCount);
            glm::vec3 H = ImportanceSampleGGX(Xi, N, roughness);
            glm::vec3 L = glm::normalize(2.0f * glm::dot(V, H) * H - V);

            float NdotL = std::max(L.z, 0.0f);
            float NdotH = std::max(H.z, 0.0f);
            float VdotH = std::max(glm::dot(V, H), 0.0f);

            if (NdotL > 0.0f)
            {
                float ggxV = NdotL * std::sqrt(NdotV * NdotV * (1.0f - a2) + a2);
                float ggxL = NdotV * std::sqrt(NdotL * NdotL * (1.0f - a2) + a2);

                float Vis = 0.5f / (ggxV + ggxL + 1e-5f);
                float G_Vis = Vis * (4.0f * NdotL * VdotH) / (NdotH);

                float Fc = std::pow(1.0f - VdotH, 5.0f);

                if (!std::isnan(G_Vis))
                {
                    A += (1.0f - Fc) * G_Vis;
                    B += Fc * G_Vis;
                }
            }
        }

        A /= static_cast<float>(_sampleCount);
        B /= static_cast<float>(_sampleCount);

        return glm::vec2(A, B);
    }
}