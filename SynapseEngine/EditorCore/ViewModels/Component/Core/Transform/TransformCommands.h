#pragma once
#include "EditorCore/Command/ICommand.h"
#include "EditorCore/Api/ITransformApi.h"
#include <glm/glm.hpp>

namespace Syn 
{
    class ChangePositionCommand : public ICommand {
    public:
        ChangePositionCommand(ITransformApi* api, EntityID entity, const glm::vec3& oldPos, const glm::vec3& newPos)
            : _api(api), _entity(entity), _oldPos(oldPos), _newPos(newPos) {}

        void Execute() override {
            _api->SetEntityPosition(_entity, _newPos);
        }

        void Undo() override {
            _api->SetEntityPosition(_entity, _oldPos);
        }

    private:
        ITransformApi* _api;
        EntityID _entity;
        glm::vec3 _oldPos;
        glm::vec3 _newPos;
    };

    class ChangeRotationCommand : public ICommand {
    public:
        ChangeRotationCommand(ITransformApi* api, EntityID entity, const glm::vec3& oldRot, const glm::vec3& newRot)
            : _api(api), _entity(entity), _oldRot(oldRot), _newRot(newRot) {}

        void Execute() override {
            _api->SetEntityRotation(_entity, _newRot);
        }

        void Undo() override {
            _api->SetEntityRotation(_entity, _oldRot);
        }

    private:
        ITransformApi* _api;
        EntityID _entity;
        glm::vec3 _oldRot;
        glm::vec3 _newRot;
    };

    class ChangeScaleCommand : public ICommand {
    public:
        ChangeScaleCommand(ITransformApi* api, EntityID entity, const glm::vec3& oldScale, const glm::vec3& newScale)
            : _api(api), _entity(entity), _oldScale(oldScale), _newScale(newScale) {}

        void Execute() override {
            _api->SetEntityScale(_entity, _newScale);
        }

        void Undo() override {
            _api->SetEntityScale(_entity, _oldScale);
        }

    private:
        ITransformApi* _api;
        EntityID _entity;
        glm::vec3 _oldScale;
        glm::vec3 _newScale;
    };

}