#pragma once
#include "ICommand.h"
#include <memory>
#include <vector>

namespace Syn {
    class CommandRegistry {
    public:
        //Todo: ServiceLocator!
        static CommandRegistry& Get() { static CommandRegistry instance; return instance; }

        void ExecuteCommand(std::shared_ptr<ICommand> command) {
            command->Execute();
            _undoStack.push_back(command);
            _redoStack.clear();
        }

        void Undo() {
            if (_undoStack.empty()) return;
            auto cmd = _undoStack.back();
            cmd->Undo();
            _undoStack.pop_back();
            _redoStack.push_back(cmd);
        }

        void Redo() {
            if (_redoStack.empty()) return;
            auto cmd = _redoStack.back();
            cmd->Execute();
            _redoStack.pop_back();
            _undoStack.push_back(cmd);
        }
    private:
        CommandRegistry() = default;
        std::vector<std::shared_ptr<ICommand>> _undoStack;
        std::vector<std::shared_ptr<ICommand>> _redoStack;
    };
}