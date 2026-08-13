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

#ifndef SYN_INCLUDES_COMMON_CAMERA_GLSL
#define SYN_INCLUDES_COMMON_CAMERA_GLSL

struct CameraComponent {
    mat4 view;
    mat4 viewInv;
    mat4 proj;
    mat4 projInv;
    mat4 projVulkan;
    mat4 projVulkanInv;
    mat4 viewProj;
    mat4 viewProjInv;
    mat4 viewProjVulkan;
    mat4 viewProjVulkanInv;
    vec4 eye;
    vec4 params; // (near, far, padding, padding)
    vec4 frustum[6];
};

layout(buffer_reference, std430) readonly restrict buffer CameraPool { 
    CameraComponent data[]; 
};

#define GET_CAMERA_POOL(addr)   CameraPool(addr)
#define GET_CAMERA(addr, idx)   CameraPool(addr).data[idx]

#define GET_CAMERA_NEAR(cam)    ((cam).params.x)
#define GET_CAMERA_FAR(cam)     ((cam).params.y)

#endif