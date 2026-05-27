#pragma once
#include <string>

namespace Syn {
    class ISceneAPI {
    public:
        virtual ~ISceneAPI() = default;

        virtual void NewScene() = 0;
        virtual void LoadScene(const std::string& filepath = "") = 0;
        virtual void SaveScene(const std::string& filepath = "") = 0;
    };
}