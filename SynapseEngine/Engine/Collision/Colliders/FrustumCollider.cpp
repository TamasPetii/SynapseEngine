#include "FrustumCollider.h"

namespace Syn
{
    void FrustumCollider::Update(const glm::mat4& mat)
    {
        // Gribb-Hartmann Algorithm
        // 0: Near, 1: Right, 2: Left, 3: Top, 4: Bottom, 5: Far

        // 0. Near plane (for Vulkan [0, 1] depth): z >= 0
        planes[0] = NormalizePlane(glm::vec4(
            mat[0][2], mat[1][2], mat[2][2], mat[3][2]
        ));

        // 1. Right plane: w - x >= 0
        planes[1] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][0], mat[1][3] - mat[1][0], mat[2][3] - mat[2][0], mat[3][3] - mat[3][0]
        ));

        // 2. Left plane: w + x >= 0
        planes[2] = NormalizePlane(glm::vec4(
            mat[0][3] + mat[0][0], mat[1][3] + mat[1][0], mat[2][3] + mat[2][0], mat[3][3] + mat[3][0]
        ));

        // 3. Top plane: w - y >= 0
        planes[3] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][1], mat[1][3] - mat[1][1], mat[2][3] - mat[2][1], mat[3][3] - mat[3][1]
        ));

        // 4. Bottom plane: w + y >= 0
        planes[4] = NormalizePlane(glm::vec4(
            mat[0][3] + mat[0][1], mat[1][3] + mat[1][1], mat[2][3] + mat[2][1], mat[3][3] + mat[3][1]
        ));

        // 5. Far plane: w - z >= 0
        planes[5] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][2], mat[1][3] - mat[1][2], mat[2][3] - mat[2][2], mat[3][3] - mat[3][2]
        ));
    }
}