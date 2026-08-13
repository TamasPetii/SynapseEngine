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

#include "Application.h"
#include "Windows/GlfwWindow.h"
#include <chrono>

namespace Syn {

    Application* Application::_instance = nullptr;

    Application::Application(const ApplicationConfig& config) {
        SYN_ASSERT(!_instance, "Application already exists!");

        _instance = this;

        WindowConfig windowConfig {
            .title = config.Name,
			.width = config.Width,
			.height = config.Height,
			.resizable = config.Resizable
        };

        _window = std::make_unique<GlfwWindow>(windowConfig);

        WindowCallbacks callbacks;

        callbacks.OnClose = [this]() {
            OnWindowClose();
            };

        callbacks.OnResize = [this](uint32_t w, uint32_t h) {
            OnResize(w, h);
            };

        callbacks.OnKey = [this](int key, int scancode, int action, int mods) {
            OnKey(key, scancode, action, mods);
            };

        callbacks.OnMouseMove = [this](float x, float y) {
            OnMouseMove(x, y);
            };

        callbacks.OnMouseButton = [this](int button, int action, int mods) {
            OnMouseButton(button, action, mods);
            };

        callbacks.OnScroll = [this](float xOffset, float yOffset) {
            OnScroll(xOffset, yOffset);
            };

        callbacks.OnChar = [this](unsigned int codepoint) {
            OnChar(codepoint);
            };

        _window->SetCallbacks(callbacks);
    }

    Application::~Application() {
        OnShutdown();
        _instance = nullptr;
    }

    void Application::Run() {
        OnInit();

        while (_isRunning) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            static auto startTime = currentTime;

            float time = std::chrono::duration<float>(currentTime - startTime).count();
            float dt = time - _lastFrameTime;
            _lastFrameTime = time;

            _window->Update();

            if (_window->ShouldClose()) {
                _isRunning = false;
            }

            if (!_isRunning) break;

            OnUpdate(dt);
            OnRender();
        }
    }

    void Application::Close() {
        _isRunning = false;
    }

    bool Application::OnWindowClose() {
        _isRunning = false;
        return true;
    }
}