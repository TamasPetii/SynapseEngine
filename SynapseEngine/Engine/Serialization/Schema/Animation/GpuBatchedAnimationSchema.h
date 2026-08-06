#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"

#include "Engine/Serialization/Schema/Models/GpuIndexedDrawDataSchema.h" 
#include "Engine/Serialization/Schema/Models/GpuMeshletDrawDataSchema.h"
#include "Engine/Serialization/Schema/Models/GpuBatchedModelSchema.h" 

#include "Engine/Animation/Data/Gpu/GpuBatchedAnimation.h"
#include "Engine/Animation/Data/Common/AnimationKeys.h"
#include "Engine/Animation/Data/Common/BoneTrack.h"
#include "Engine/Animation/Data/Common/AnimationNode.h"

namespace Syn
{
    template <>
    struct Schema<AnimationNode> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& n = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", n.name);
            ar.Property("parentIndex", n.parentIndex);
            ar.Property("trackIndex", n.trackIndex);
            ar.Property("offsetMatrix", n.offsetMatrix);
            ar.Property("defaultLocalTransform", n.defaultLocalTransform);
        }
    };

    template <> 
    struct Schema<AnimationKeyPosition> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("time", v.time); 
            ar.Property("value", v.value);
        }
    };

    template <> 
    struct Schema<AnimationKeyRotation> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("time", v.time); 
            ar.Property("value", v.value);
        }
    };
    template <> 
    struct Schema<AnimationKeyScale> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("time", v.time); 
            ar.Property("value", v.value);
        }
    };

    template <> 
    struct Schema<BoneTrack> {
        static constexpr bool exists = true;

        template <typename Archive, typename U> 
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& t = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("nodeName", t.nodeName);

            if (ar.IsBinary()) {
                BlitVector<AnimationKeyPosition> pos{ t.positions };
                ar.Property("positions", pos);
                BlitVector<AnimationKeyRotation> rot{ t.rotations };
                ar.Property("rotations", rot);
                BlitVector<AnimationKeyScale> scl{ t.scales };
                ar.Property("scales", scl);
            }
            else {
                ar.Property("positions", t.positions);
                ar.Property("rotations", t.rotations);
                ar.Property("scales", t.scales);
            }
        }
    };


    template <> 
    struct Schema<GpuAnimationDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("frameCount", d.frameCount);
            ar.Property("nodeCount", d.nodeCount);
            ar.Property("globalVertexCount", d.globalVertexCount);
            ar.Property("globalMeshCount", d.globalMeshCount);
            ar.Property("globalMeshletCount", d.globalMeshletCount);
            ar.Property("durationInSeconds", d.durationInSeconds);
            ar.Property("sampleRate", d.sampleRate);
        }
    };

    template <> 
    struct Schema<VertexSkinData> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("boneIndices", v.boneIndices);
            ar.Property("boneWeights", v.boneWeights);
        }
    };

    template <> 
    struct Schema<GpuBatchedAnimation> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& a = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("descriptor", a.descriptor);
            ar.Property("globalCollider", a.globalCollider);

            if (ar.IsBinary()) {
                BlitVector<VertexSkinData> vertexSkinData{ a.vertexSkinData };
                ar.Property("vertexSkinData", vertexSkinData);

                BlitVector<GpuNodeTransform> nodeTransforms{ a.nodeTransforms };
                ar.Property("nodeTransforms", nodeTransforms);

                BlitVector<GpuMeshCollider> frameGlobalColliders{ a.frameGlobalColliders };
                ar.Property("frameGlobalColliders", frameGlobalColliders);

                BlitVector<GpuMeshCollider> frameMeshColliders{ a.frameMeshColliders };
                ar.Property("frameMeshColliders", frameMeshColliders);

                BlitVector<GpuMeshletCollider> frameMeshletColliders{ a.frameMeshletColliders };
                ar.Property("frameMeshletColliders", frameMeshletColliders);
            }
            else {
                ar.Property("vertexSkinData", a.vertexSkinData);
                ar.Property("nodeTransforms", a.nodeTransforms);
                ar.Property("frameGlobalColliders", a.frameGlobalColliders);
                ar.Property("frameMeshColliders", a.frameMeshColliders);
                ar.Property("frameMeshletColliders", a.frameMeshletColliders);
            }

            ar.Property("tracks", a.tracks);
            ar.Property("nodes", a.nodes);
        }
    };
}