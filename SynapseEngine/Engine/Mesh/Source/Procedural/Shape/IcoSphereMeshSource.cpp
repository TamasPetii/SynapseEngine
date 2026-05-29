#include "IcoSphereMeshSource.h"
#include <glm/gtc/constants.hpp>

namespace Syn
{
    IcoSphereMeshSource::IcoSphereMeshSource(float radius, uint32_t subdivisions)
        : ShapeMeshSource("IcoSphere"), _radius(radius), _subdivisions(subdivisions)
    {}

    uint32_t IcoSphereMeshSource::GetMiddlePoint(uint32_t p1, uint32_t p2, std::map<int64_t, uint32_t>& cache)
    {
        bool firstIsSmaller = p1 < p2;
        int64_t smallerIndex = firstIsSmaller ? p1 : p2;
        int64_t greaterIndex = firstIsSmaller ? p2 : p1;
        int64_t key = (smallerIndex << 32) + greaterIndex;

        if (cache.find(key) != cache.end()) {
            return cache[key];
        }

        glm::vec3 point1 = _cachedPositions[p1];
        glm::vec3 point2 = _cachedPositions[p2];
        glm::vec3 middle = glm::normalize(point1 + point2) * _radius;

        uint32_t index = static_cast<uint32_t>(_cachedPositions.size());
        _cachedPositions.push_back(middle);
        cache[key] = index;
        return index;
    }

    void IcoSphereMeshSource::GenerateGeometry()
    {
        if (_isGenerated) return;

        const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

        _cachedPositions = {
            glm::normalize(glm::vec3(-1.0f,  t,  0.0f)) * _radius,
            glm::normalize(glm::vec3(1.0f,  t,  0.0f)) * _radius,
            glm::normalize(glm::vec3(-1.0f, -t,  0.0f)) * _radius,
            glm::normalize(glm::vec3(1.0f, -t,  0.0f)) * _radius,
            glm::normalize(glm::vec3(0.0f, -1.0f,  t)) * _radius,
            glm::normalize(glm::vec3(0.0f,  1.0f,  t)) * _radius,
            glm::normalize(glm::vec3(0.0f, -1.0f, -t)) * _radius,
            glm::normalize(glm::vec3(0.0f,  1.0f, -t)) * _radius,
            glm::normalize(glm::vec3(t,  0.0f, -1.0f)) * _radius,
            glm::normalize(glm::vec3(t,  0.0f,  1.0f)) * _radius,
            glm::normalize(glm::vec3(-t,  0.0f, -1.0f)) * _radius,
            glm::normalize(glm::vec3(-t,  0.0f,  1.0f)) * _radius
        };

        _cachedIndices = {
            0, 11, 5,   0, 5, 1,    0, 1, 7,    0, 7, 10,   0, 10, 11,
            1, 5, 9,    5, 11, 4,   11, 10, 2,  10, 7, 6,   7, 1, 8,
            3, 9, 4,    3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
            4, 9, 5,    2, 4, 11,   6, 2, 10,   8, 6, 7,    9, 8, 1
        };

        std::map<int64_t, uint32_t> cache;
        for (uint32_t i = 0; i < _subdivisions; i++) {
            std::vector<uint32_t> newIndices;
            newIndices.reserve(_cachedIndices.size() * 4);

            for (size_t j = 0; j < _cachedIndices.size(); j += 3) {
                uint32_t v1 = _cachedIndices[j];
                uint32_t v2 = _cachedIndices[j + 1];
                uint32_t v3 = _cachedIndices[j + 2];

                uint32_t a = GetMiddlePoint(v1, v2, cache);
                uint32_t b = GetMiddlePoint(v2, v3, cache);
                uint32_t c = GetMiddlePoint(v3, v1, cache);

                newIndices.insert(newIndices.end(), { v1, a, c, v2, b, a, v3, c, b, a, b, c });
            }
            _cachedIndices = newIndices;
        }

        _cachedUVs.resize(_cachedPositions.size());
        _cachedNormals.resize(_cachedPositions.size());

        for (size_t i = 0; i < _cachedPositions.size(); ++i) {
            glm::vec3 normalizedPos = glm::normalize(_cachedPositions[i]);
            _cachedNormals[i] = normalizedPos;

            float u = 0.5f + (std::atan2(normalizedPos.z, normalizedPos.x) / (2.0f * glm::pi<float>()));
            float v = 0.5f - (std::asin(normalizedPos.y) / glm::pi<float>());

            _cachedUVs[i] = glm::vec2(u, v);
        }

        _isGenerated = true;
    }

    void IcoSphereMeshSource::GeneratePositions(std::vector<glm::vec3>& outPositions)
    {
        GenerateGeometry();
        outPositions = _cachedPositions;
    }

    void IcoSphereMeshSource::GenerateIndices(std::vector<uint32_t>& outIndices)
    {
        GenerateGeometry();
        outIndices = _cachedIndices;
    }

    void IcoSphereMeshSource::GenerateUVs(std::span<glm::vec2> outUVs)
    {
        GenerateGeometry();
        for (size_t i = 0; i < outUVs.size() && i < _cachedUVs.size(); ++i) {
            outUVs[i] = _cachedUVs[i];
        }
    }

    void IcoSphereMeshSource::GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals)
    {
        GenerateGeometry();
        for (size_t i = 0; i < outNormals.size() && i < _cachedNormals.size(); ++i) {
            outNormals[i] = _cachedNormals[i];
        }
    }
}