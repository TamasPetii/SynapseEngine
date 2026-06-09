#pragma once
#include "ICommand.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    template <typename ApiType, typename ValueType, auto SetterFunc>
    class ComponentChangeCommand : public ICommand {
    public:
        ComponentChangeCommand(ApiType* api, EntityID entity, const ValueType& oldVal, const ValueType& newVal)
            : _api(api), _entity(entity), _oldVal(oldVal), _newVal(newVal) {}

        void Execute() override {
            (_api->*SetterFunc)(_entity, _newVal);
        }

        void Undo() override {
            (_api->*SetterFunc)(_entity, _oldVal);
        }

    private:
        ApiType* _api;
        EntityID _entity;
        ValueType _oldVal;
        ValueType _newVal;
    };
}