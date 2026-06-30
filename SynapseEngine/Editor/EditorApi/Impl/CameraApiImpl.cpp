#include "CameraApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Core/CameraComponent.h"

namespace Syn {

    bool CameraApiImpl::HasCamera(EntityID entity) const {
        return EditorApiUtils::HasComponent<CameraComponent>(_sceneManager, entity);
    }

    float CameraApiImpl::GetCameraYaw(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.yaw; }, 0.0f);
    }
    float CameraApiImpl::GetCameraPitch(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.pitch; }, 0.0f);
    }
    float CameraApiImpl::GetCameraNearPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.nearPlane; }, 0.1f);
    }
    float CameraApiImpl::GetCameraFarPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.farPlane; }, 1000.0f);
    }
    float CameraApiImpl::GetCameraFov(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.fov; }, 45.0f);
    }
    float CameraApiImpl::GetCameraSpeed(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.speed; }, 1.0f);
    }
    float CameraApiImpl::GetCameraSensitivity(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.sensitivity; }, 0.1f);
    }
    float CameraApiImpl::GetCameraDistance(EntityID entity) const {
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, entity, [](const auto& c) { return c.distance; }, 10.0f);
    }

    void CameraApiImpl::SetCameraYaw(EntityID entity, float yaw) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.yaw = yaw; });
    }
    void CameraApiImpl::SetCameraPitch(EntityID entity, float pitch) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.pitch = pitch; });
    }
    void CameraApiImpl::SetCameraNearPlane(EntityID entity, float nearPlane) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.nearPlane = nearPlane; });
    }
    void CameraApiImpl::SetCameraFarPlane(EntityID entity, float farPlane) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.farPlane = farPlane; });
    }
    void CameraApiImpl::SetCameraFov(EntityID entity, float fov) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.fov = fov; });
    }
    void CameraApiImpl::SetCameraSpeed(EntityID entity, float speed) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.speed = speed; });
    }
    void CameraApiImpl::SetCameraSensitivity(EntityID entity, float sensitivity) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.sensitivity = sensitivity; });
    }
    void CameraApiImpl::SetCameraDistance(EntityID entity, float distance) {
        EditorApiUtils::ModifyComponent<CameraComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.distance = distance; });
    }
}