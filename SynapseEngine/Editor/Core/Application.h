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

#include "Engine/SynMacro.h"
#include "Windows/Window.h"
#include <memory>

namespace Syn {

    struct ApplicationConfig {
        const char* Name = "SynapseApplication";
        uint32_t Width = 1280;
        uint32_t Height = 720;
        bool Resizable = true;
    };

    class Application {
    public:
        Application(const ApplicationConfig& config);
        virtual ~Application();

        void Run();
        void Close();

        SYN_INLINE Window& GetWindow() const { return *_window; }
        static Application& Get() { return *_instance; }
    protected:
        virtual void OnInit() {}
        virtual void OnUpdate(float dt) {}
        virtual void OnRender() {}
        virtual void OnShutdown() {}

        virtual void OnResize(uint32_t width, uint32_t height) {}
        virtual void OnKey(int key, int scancode, int action, int mods) {}
        virtual void OnMouseMove(float x, float y) {}
        virtual void OnMouseButton(int button, int action, int mods) {}
        virtual void OnScroll(float xOffset, float yOffset) {}
        virtual void OnChar(unsigned int codepoint) {}
    private:
        bool OnWindowClose();
    private:
        static Application* _instance;
        bool _isRunning = true;
        float _lastFrameTime = 0.0f;
        std::unique_ptr<Window> _window;
    };

    Application* CreateApplication();
}