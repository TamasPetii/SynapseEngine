#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Syn
{
    template <>
    struct Schema<glm::vec2> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val)
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
        }
    };

    template <>
    struct Schema<glm::uvec2> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val)
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
        }
    };

    template <>
    struct Schema<glm::vec3> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
            ar.Property("z", val.z);
        }
    };

    template <>
    struct Schema<glm::uvec3> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val)
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
            ar.Property("z", val.z);
        }
    };

    template <>
    struct Schema<glm::vec4> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
            ar.Property("z", val.z);
            ar.Property("w", val.w);
        }
    };

    template <>
    struct Schema<glm::uvec4> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val)
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
            ar.Property("z", val.z);
            ar.Property("w", val.w);
        }
    };

    template <>
    struct Schema<glm::quat> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("w", val.w);
            ar.Property("x", val.x);
            ar.Property("y", val.y);
            ar.Property("z", val.z);
        }
    };

    template <>
    struct Schema<glm::mat4> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            ar.Property("col0", val[0]);
            ar.Property("col1", val[1]);
            ar.Property("col2", val[2]);
            ar.Property("col3", val[3]);
        }
    };
}