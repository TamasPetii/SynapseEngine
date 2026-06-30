#pragma once
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ICameraApi {
    public:
        virtual ~ICameraApi() = default;

        virtual bool HasCamera(EntityID entity) const = 0;

        virtual float GetCameraYaw(EntityID entity) const = 0;
        virtual float GetCameraPitch(EntityID entity) const = 0;
        virtual float GetCameraNearPlane(EntityID entity) const = 0;
        virtual float GetCameraFarPlane(EntityID entity) const = 0;
        virtual float GetCameraFov(EntityID entity) const = 0;
        virtual float GetCameraSpeed(EntityID entity) const = 0;
        virtual float GetCameraSensitivity(EntityID entity) const = 0;
        virtual float GetCameraDistance(EntityID entity) const = 0;

        virtual void SetCameraYaw(EntityID entity, float yaw) = 0;
        virtual void SetCameraPitch(EntityID entity, float pitch) = 0;
        virtual void SetCameraNearPlane(EntityID entity, float nearPlane) = 0;
        virtual void SetCameraFarPlane(EntityID entity, float farPlane) = 0;
        virtual void SetCameraFov(EntityID entity, float fov) = 0;
        virtual void SetCameraSpeed(EntityID entity, float speed) = 0;
        virtual void SetCameraSensitivity(EntityID entity, float sensitivity) = 0;
        virtual void SetCameraDistance(EntityID entity, float distance) = 0;
    };
}