#pragma once

namespace Syn {
    struct CameraState {
        bool hasComponent = false;

        float yaw;
        float pitch;
        float nearPlane;
        float farPlane;
        float fov;
        float speed;
        float sensitivity;
        float distance;
    };
}