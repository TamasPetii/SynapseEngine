#include "EditorApiImpl.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    void EditorApiImpl::NewScene() {
        Syn::Info("EditorApiImpl: New Scene intent triggered.");
    }

    void EditorApiImpl::LoadScene(const std::string& filepath) {
        if (filepath.empty()) {
            _sceneManager->LoadSceneFromFile("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.json");
            //_sceneManager->LoadSceneFromFile("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.yaml");
            // _sceneManager->LoadSceneFromFile("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.toml");
            // _sceneManager->LoadSceneFromFile("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.xml");
            // _sceneManager->LoadSceneFromFile("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.bin");

            Syn::Info("EditorApiImpl: Scene dummy load triggered from Desktop.");
        }
        else {
            _sceneManager->LoadSceneFromFile(filepath);
            Syn::Info("EditorApiImpl: Scene loaded from {}", filepath);
        }
    }

    void EditorApiImpl::SaveScene(const std::string& filepath) {
        auto activeScene = _sceneManager->GetActiveScene();

        if (!activeScene) return;

        if (filepath.empty()) {
            _sceneManager->SaveActiveScene("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.json");
            //_sceneManager->SaveActiveScene("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.yaml");
            //_sceneManager->SaveActiveScene("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.toml");
            //_sceneManager->SaveActiveScene("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.xml");
            //_sceneManager->SaveActiveScene("C:\\Users\\User\\Desktop\\SceneSave\\TestLevel.bin");

            Syn::Info("EditorApiImpl: Scene dummy save triggered to Desktop.");
        }
        else {
            _sceneManager->SaveActiveScene(filepath);
            Syn::Info("EditorApiImpl: Scene saved to {}", filepath);
        }
    }
}