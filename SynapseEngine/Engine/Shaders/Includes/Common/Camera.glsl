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