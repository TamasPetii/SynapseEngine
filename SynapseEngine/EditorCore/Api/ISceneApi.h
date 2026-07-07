#pragma once
#include <string>

namespace Syn {
    class ISceneApi {
    public:
        virtual ~ISceneApi() = default;

        virtual void NewScene() = 0;
        virtual void LoadScene(const std::string& filepath = "") = 0;
        virtual void SaveScene(const std::string& filepath = "") = 0;
        virtual void ActivateScene(const std::string& sceneName) = 0;
    };
}