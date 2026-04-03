#pragma once
#include "Engine/SynApi.h"
#include "CylinderMeshSource.h"

namespace Syn
{
    class SYN_API ConeMeshSource : public CylinderMeshSource
    {
    public:
        ConeMeshSource(float radius = 1.0f, float height = 2.0f, uint32_t radialSegments = 32, uint32_t heightSegments = 1)
            : CylinderMeshSource(radius, 0.0f, height, radialSegments, heightSegments)
        {
            _name = "Cone";
        }

        virtual ~ConeMeshSource() override = default;
    };
}