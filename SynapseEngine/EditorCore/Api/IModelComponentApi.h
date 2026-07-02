#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    class IModelComponentApi {
    public:
        virtual ~IModelComponentApi() = default;

        virtual bool HasModelComponent(EntityID entity) const = 0;

        virtual bool GetCastShadow(EntityID entity) const = 0;
        virtual bool GetReceiveShadow(EntityID entity) const = 0;
        virtual uint32_t GetModelIndex(EntityID entity) const = 0;

        virtual void SetCastShadow(EntityID entity, bool cast) = 0;
        virtual void SetReceiveShadow(EntityID entity, bool receive) = 0;
        virtual void SetModelIndex(EntityID entity, uint32_t index) = 0;

        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailableModels() const = 0;
    };
}