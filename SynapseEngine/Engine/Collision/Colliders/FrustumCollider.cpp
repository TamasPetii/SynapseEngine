#include "FrustumCollider.h"

namespace Syn
{
    void FrustumCollider::Update(const glm::mat4& viewProj)
    {
        auto normalizePlane = [](glm::vec4 p) -> glm::vec4 {
            float length = glm::length(glm::vec3(p));
            return p / length;
            };

        glm::mat4 m = glm::transpose(viewProj);

        const glm::vec4& row0 = m[0];
        const glm::vec4& row1 = m[1];
        const glm::vec4& row2 = m[2];
        const glm::vec4& row3 = m[3];

        // Gribb-Hartmann
        planes[0] = normalizePlane(row2);
        planes[1] = normalizePlane(row3 - row2);
        planes[2] = normalizePlane(row3 + row0);
        planes[3] = normalizePlane(row3 - row0);
        planes[4] = normalizePlane(row3 - row1);
        planes[5] = normalizePlane(row3 + row1);
    }
}