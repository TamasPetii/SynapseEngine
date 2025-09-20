#include "Cone.h"

Cone::Cone(uint32_t segments, float radius, float height)
    : segments(segments), radius(radius), height(height)
{
    Initialize();
}

void Cone::PopulateSurfacePoints()
{
    float halfHeight = height * 0.5f;

    // Base ring points (at y = -halfHeight)
    for (uint32_t j = 0; j <= segments; ++j)
    {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / segments;
        glm::vec3 point;
        point.x = radius * cosf(theta);
        point.y = -halfHeight;
        point.z = radius * sinf(theta);
        surfacePoints.push_back(point);
    }

    // Apex point (y = +halfHeight)
    surfacePoints.push_back({ 0.0f, halfHeight, 0.0f });

    // Base center point (y = -halfHeight)
    surfacePoints.push_back({ 0.0f, -halfHeight, 0.0f });
}

void Cone::GenerateVertices()
{
    float halfHeight = height * 0.5f;

    // Side vertices (base ring + apex)
    for (uint32_t j = 0; j <= segments; ++j)
    {
        Vertex vertex;
        vertex.position = surfacePoints[j];  // Base ring

        // Normal for side: normalize vector from apex to base point
        glm::vec3 toApex = glm::vec3(0.0f, halfHeight, 0.0f) - vertex.position;
        glm::vec3 radial = glm::normalize(glm::vec3(vertex.position.x, 0.0f, vertex.position.z));
        vertex.normal = glm::normalize(glm::cross(glm::cross(toApex, radial), toApex));

        vertex.uv_x = static_cast<float>(j) / segments;
        vertex.uv_y = 0.0f;
        vertices.push_back(vertex);
    }

    // Apex vertex
    uint32_t apexIdx = segments + 1;
    {
        Vertex vertex;
        vertex.position = surfacePoints[apexIdx];
        vertex.normal = { 0.0f, 1.0f, 0.0f };  // Upward
        vertex.uv_x = 0.5f;
        vertex.uv_y = 1.0f;
        vertices.push_back(vertex);
    }

    // Base cap vertices
    uint32_t baseCenterIdx = apexIdx + 1;
    for (uint32_t j = 0; j <= segments; ++j)
    {
        Vertex vertex;
        vertex.position = surfacePoints[j];  // Base ring
        vertex.normal = { 0.0f, -1.0f, 0.0f }; // Downward
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / segments;
        vertex.uv_x = 0.5f + 0.5f * cosf(theta);
        vertex.uv_y = 0.5f + 0.5f * sinf(theta);
        vertices.push_back(vertex);
    }

    // Base center
    {
        Vertex vertex;
        vertex.position = surfacePoints[baseCenterIdx];
        vertex.normal = { 0.0f, -1.0f, 0.0f }; // Downward
        vertex.uv_x = 0.5f;
        vertex.uv_y = 0.5f;
        vertices.push_back(vertex);
    }
}

void Cone::GenerateIndices()
{
    uint32_t apexIdx = segments + 1;
    for (uint32_t j = 0; j < segments; ++j)
    {
        indices.push_back(j);
        indices.push_back(apexIdx);
        indices.push_back(j + 1);
    }

    uint32_t baseStart = apexIdx + 1;
    uint32_t baseCenter = baseStart + (segments + 1);
    for (uint32_t j = 0; j < segments; ++j)
    {
        indices.push_back(baseStart + j + 1);
        indices.push_back(baseCenter);
        indices.push_back(baseStart + j);
    }
}