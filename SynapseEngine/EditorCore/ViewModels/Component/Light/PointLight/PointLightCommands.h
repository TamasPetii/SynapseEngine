#pragma once
#include "EditorCore/Command/ICommand.h"
#include "EditorCore/Api/IPointLightApi.h"
#include <glm/glm.hpp>

namespace Syn 
{
    class ChangePointLightColorCommand : public ICommand {
    public:
        ChangePointLightColorCommand(IPointLightApi* api, EntityID entity, const glm::vec3& oldColor, const glm::vec3& newColor)
            : _api(api), _entity(entity), _oldColor(oldColor), _newColor(newColor) {}
        void Execute() override { _api->SetLightColor(_entity, _newColor); }
        void Undo() override { _api->SetLightColor(_entity, _oldColor); }
    private:
        IPointLightApi* _api; EntityID _entity; glm::vec3 _oldColor, _newColor;
    };

    class ChangePointLightStrengthCommand : public ICommand {
    public:
        ChangePointLightStrengthCommand(IPointLightApi* api, EntityID entity, float oldVal, float newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}
        void Execute() override { _api->SetLightStrength(_entity, _newVal); }
        void Undo() override { _api->SetLightStrength(_entity, _oldVal); }
    private:
        IPointLightApi* _api; EntityID _entity; float _oldVal, _newVal;
    };

    class ChangePointLightRadiusCommand : public ICommand {
    public:
        ChangePointLightRadiusCommand(IPointLightApi* api, EntityID entity, float oldVal, float newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}
        void Execute() override { _api->SetLightRadius(_entity, _newVal); }
        void Undo() override { _api->SetLightRadius(_entity, _oldVal); }
    private:
        IPointLightApi* _api; EntityID _entity; float _oldVal, _newVal;
    };

    class ChangePointLightWeakenCommand : public ICommand {
    public:
        ChangePointLightWeakenCommand(IPointLightApi* api, EntityID entity, float oldVal, float newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}
        void Execute() override { _api->SetLightWeakenDistance(_entity, _newVal); }
        void Undo() override { _api->SetLightWeakenDistance(_entity, _oldVal); }
    private:
        IPointLightApi* _api; EntityID _entity; float _oldVal, _newVal;
    };

    class ChangePointLightShadowNearCommand : public ICommand {
    public:
        ChangePointLightShadowNearCommand(IPointLightApi* api, EntityID entity, float oldVal, float newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}
        void Execute() override { _api->SetShadowNearPlane(_entity, _newVal); }
        void Undo() override { _api->SetShadowNearPlane(_entity, _oldVal); }
    private:
        IPointLightApi* _api; EntityID _entity; float _oldVal, _newVal;
    };

    class ChangePointLightShadowFarCommand : public ICommand {
    public:
        ChangePointLightShadowFarCommand(IPointLightApi* api, EntityID entity, float oldVal, float newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}
        void Execute() override { _api->SetShadowFarPlane(_entity, _newVal); }
        void Undo() override { _api->SetShadowFarPlane(_entity, _oldVal); }
    private:
        IPointLightApi* _api; EntityID _entity; float _oldVal, _newVal;
    };
}