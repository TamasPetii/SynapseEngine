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

#ifndef SYN_INCLUDES_COMMON_TRANSFORM_GLSL
#define SYN_INCLUDES_COMMON_TRANSFORM_GLSL

#include "../Core.glsl"

struct TransformComponent {
    mat4 transform;
    mat4 transformIT;
};

struct GpuNodeTransform {
    mat4 globalTransform;
    mat4 globalTransformIT;
};

struct TransformModelLink
{
    uint entityIndex;
    uint modelDenseIndex;
};

layout(buffer_reference, std430) readonly restrict buffer TransformPool { 
    TransformComponent data[]; 
};

layout(buffer_reference, std430) readonly restrict buffer NodeBuffer { 
    GpuNodeTransform data[];
};

layout(buffer_reference, std430) readonly restrict buffer TransformModelLinkBuffer { 
    TransformModelLink data[];
};


#define GET_TRANSFORM_POOL(addr)        TransformPool(addr)
#define GET_TRANSFORM(addr, idx)        TransformPool(addr).data[idx]
#define GET_NODE_TRANSFORM(addr, idx)   NodeBuffer(addr).data[idx]
#define GET_TRANSFORM_MODEL_LINK(addr, idx)   TransformModelLinkBuffer(addr).data[idx]

#endif