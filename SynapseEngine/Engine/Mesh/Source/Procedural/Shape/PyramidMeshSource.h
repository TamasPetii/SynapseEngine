#pragma once
#include "Engine/SynApi.h"
#include "CylinderMeshSource.h"

namespace Syn
{
    class SYN_API PyramidMeshSource : public CylinderMeshSource
    {
    public:
        PyramidMeshSource(float baseRadius = 1.0f, float height = 2.0f, uint32_t heightSegments = 1)
            : CylinderMeshSource(baseRadius, 0.0f, height, 4, heightSegments)
        {
            _name = "Pyramid";
        }

        virtual ~PyramidMeshSource() override = default;
    };
}