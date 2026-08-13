// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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