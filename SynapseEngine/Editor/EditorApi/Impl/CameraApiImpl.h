#pragma once
#include "EditorCore/Api/ICameraApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class CameraApiImpl : public ICameraApi {
    public:
        CameraApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasCamera(EntityID entity) const override;

        bool GetCameraIsOrthographic(EntityID entity) const override;
        float GetCameraOrthoSize(EntityID entity) const override;
        bool GetCameraUseOrbit(EntityID entity) const override;

        float GetCameraYaw(EntityID entity) const override;
        float GetCameraPitch(EntityID entity) const override;
        float GetCameraNearPlane(EntityID entity) const override;
        float GetCameraFarPlane(EntityID entity) const override;
        float GetCameraFov(EntityID entity) const override;
        float GetCameraSpeed(EntityID entity) const override;
        float GetCameraSensitivity(EntityID entity) const override;
        float GetCameraDistance(EntityID entity) const override;

        void SetCameraIsOrthographic(EntityID entity, bool isOrthographic) override;
        void SetCameraOrthoSize(EntityID entity, float orthoSize) override;
        void SetCameraUseOrbit(EntityID entity, bool useOrbit) override;

        void SetCameraYaw(EntityID entity, float yaw) override;
        void SetCameraPitch(EntityID entity, float pitch) override;
        void SetCameraNearPlane(EntityID entity, float nearPlane) override;
        void SetCameraFarPlane(EntityID entity, float farPlane) override;
        void SetCameraFov(EntityID entity, float fov) override;
        void SetCameraSpeed(EntityID entity, float speed) override;
        void SetCameraSensitivity(EntityID entity, float sensitivity) override;
        void SetCameraDistance(EntityID entity, float distance) override;

    private:
        SceneManager* _sceneManager;
    };
}