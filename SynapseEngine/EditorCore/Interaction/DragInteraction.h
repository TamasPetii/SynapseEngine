#pragma once
#include "EditorCore/Command/CommandRegistry.h"
#include <memory>

namespace Syn 
{
    template <typename T>
    class DragInteraction {
    public:
        template <typename LiveUpdateFunc, typename CreateCommandFunc>
        void Handle(bool intentIsDragging, const T& newValue, T& stateRef, LiveUpdateFunc liveUpdate, CreateCommandFunc createCommand)
        {
            if (intentIsDragging && !_isDragging) {
                _startValue = stateRef;
                _isDragging = true;
            }

            stateRef = newValue;

            if (intentIsDragging) {
                liveUpdate(newValue);
            }
            else {
                _isDragging = false;

                auto cmd = createCommand(_startValue, newValue);

                if (cmd) {
                    CommandRegistry::Get().ExecuteCommand(cmd);
                }
            }
        }

        bool IsDragging() const { return _isDragging; }
    private:
        bool _isDragging = false;
        T _startValue{};
    };
}