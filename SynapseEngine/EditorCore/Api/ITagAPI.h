#pragma once
#include <string>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ITagAPI {
    public:
        virtual ~ITagAPI() = default;

        virtual std::string GetEntityName(EntityID entity) const = 0;
        virtual void SetEntityName(EntityID entity, const std::string& name) = 0;

        virtual std::string GetEntityTag(EntityID entity) const = 0;
        virtual void SetEntityTag(EntityID entity, const std::string& tag) = 0;

        virtual bool IsEntityEnabled(EntityID entity) const = 0;
        virtual void SetEntityEnabled(EntityID entity, bool enabled) = 0;
    };
}