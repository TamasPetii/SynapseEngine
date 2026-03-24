#pragma once
#include "EditorCore/Command/ICommand.h"
#include "EditorCore/Api/ITransformAPI.h"
#include <glm/glm.hpp>

namespace Syn 
{
    class ChangePositionCommand : public ICommand {
    public:
        ChangePositionCommand(ITransformAPI* api, EntityID entity, const glm::vec3& oldPos, const glm::vec3& newPos)
            : _api(api), _entity(entity), _oldPos(oldPos), _newPos(newPos) {}

        void Execute() override {
            _api->SetEntityPosition(_entity, _newPos);
        }

        void Undo() override {
            _api->SetEntityPosition(_entity, _oldPos);
        }

    private:
        ITransformAPI* _api;
        EntityID _entity;
        glm::vec3 _oldPos;
        glm::vec3 _newPos;
    };

    class ChangeRotationCommand : public ICommand {
    public:
        ChangeRotationCommand(ITransformAPI* api, EntityID entity, const glm::vec3& oldRot, const glm::vec3& newRot)
            : _api(api), _entity(entity), _oldRot(oldRot), _newRot(newRot) {}

        void Execute() override {
            _api->SetEntityRotation(_entity, _newRot);
        }

        void Undo() override {
            _api->SetEntityRotation(_entity, _oldRot);
        }

    private:
        ITransformAPI* _api;
        EntityID _entity;
        glm::vec3 _oldRot;
        glm::vec3 _newRot;
    };

    class ChangeScaleCommand : public ICommand {
    public:
        ChangeScaleCommand(ITransformAPI* api, EntityID entity, const glm::vec3& oldScale, const glm::vec3& newScale)
            : _api(api), _entity(entity), _oldScale(oldScale), _newScale(newScale) {}

        void Execute() override {
            _api->SetEntityScale(_entity, _newScale);
        }

        void Undo() override {
            _api->SetEntityScale(_entity, _oldScale);
        }

    private:
        ITransformAPI* _api;
        EntityID _entity;
        glm::vec3 _oldScale;
        glm::vec3 _newScale;
    };

}