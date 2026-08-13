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
#include "Core/Application.h"
#include "Engine/Engine.h"
#include "Manager/GuiManager.h"
#include "Dispatcher/InputDispatcher.h"
#include "Editor/Manager/IconManager.h"
#include "Editor/EditorApi/EditorContext.h"
#include <memory>

class Synapse : public Syn::Application {
public:
    Synapse(const Syn::ApplicationConfig& config);
    ~Synapse() override;

    void OnInit() override;
    void OnUpdate(float dt) override;
    void OnRender() override;

    void OnKey(int key, int scancode, int action, int mods) override;
    void OnMouseButton(int button, int action, int mods) override;
    void OnMouseMove(float x, float y) override;
    void OnResize(uint32_t width, uint32_t height) override;
    void OnScroll(float xOffset, float yOffset) override;
    void OnChar(unsigned int codepoint) override;
private:
    std::unique_ptr<Syn::Engine> _engine;
    std::unique_ptr<Syn::GuiManager> _guiManager;
    std::unique_ptr<Syn::InputDispatcher> _inputDispatcher;
    std::unique_ptr<Syn::IconManager> _iconManager;
    std::unique_ptr<Syn::EditorContext> _editorContext;
};