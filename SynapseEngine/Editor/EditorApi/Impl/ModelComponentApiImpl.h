#pragma once
#include "EditorCore/Api/IModelComponentApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class ModelComponentApiImpl : public IModelComponentApi {
    public:
        ModelComponentApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasModelComponent(EntityID entity) const override;

        bool GetCastShadow(EntityID entity) const override;
        bool GetReceiveShadow(EntityID entity) const override;
        uint32_t GetModelIndex(EntityID entity) const override;

        void SetCastShadow(EntityID entity, bool cast) override;
        void SetReceiveShadow(EntityID entity, bool receive) override;
        void SetModelIndex(EntityID entity, uint32_t index) override;

        std::vector<std::pair<uint32_t, std::string>> GetAvailableModels() const override;
    private:
        SceneManager* _sceneManager;
    };
}