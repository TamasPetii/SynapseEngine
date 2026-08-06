#pragma once
#include <vector>
#include <memory>
#include "Editor/Workspace/IGuiWindow.h"

namespace Syn 
{
    enum class EditorWorkspace {
        Scene,
        Model,
        Material,
        Texture,
        Animation
    };

    class IWorkspace {
    public:
        virtual ~IWorkspace() = default;

        virtual void Initialize() = 0;
        virtual void OnActivate() {}
        virtual void OnDeactivate() {}

        virtual void UpdateAndDraw() {
            for (auto& window : _windows) {
                window->UpdateAndDraw();
            }
        }

    protected:

        template<typename TWindow, typename... Args>
        void AddWindow(Args&&... args) {
            _windows.push_back(std::make_unique<TWindow>(std::forward<Args>(args)...));
        }

        std::vector<std::unique_ptr<IGuiWindow>> _windows;
    };
}