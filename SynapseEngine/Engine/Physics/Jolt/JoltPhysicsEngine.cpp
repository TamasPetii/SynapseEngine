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

#include "JoltPhysicsEngine.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Core/JobSystemWithBarrier.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include "JoltLineExtractor.h"
#include "JobSystemTaskflow.h"
#include "Engine/ServiceLocator.h"
#include <memory>

namespace Syn
{
    static JPH::EMotionType GetJoltMotionType(PhysicsMotionType type) {
        switch (type) {
        case PhysicsMotionType::Static:    return JPH::EMotionType::Static;
        case PhysicsMotionType::Kinematic: return JPH::EMotionType::Kinematic;
        case PhysicsMotionType::Dynamic:   return JPH::EMotionType::Dynamic;
        default:                           return JPH::EMotionType::Dynamic;
        }
    }

    JoltPhysicsEngine::JoltPhysicsEngine() {}

    JoltPhysicsEngine::~JoltPhysicsEngine() = default;

    void JoltPhysicsEngine::Init(const PhysicsInitParams& params)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(params.tempAllocatorSizeMB * 1024 * 1024);

        jobSystem = std::make_unique<JobSystemTaskflow>(
            *ServiceLocator::Get<tf::Executor>(),
            JPH::cMaxPhysicsJobs,
            JPH::cMaxPhysicsBarriers
        );

        physicsSystem = std::make_unique<JPH::PhysicsSystem>();

        physicsSystem->Init(
            params.maxBodies,
            params.numBodyMutexes,
            params.maxBodyPairs,
            params.maxContactConstraints,
            bpLayerInterface,
            objVsBpFilter,
            objPairFilter
        );
    }

    void JoltPhysicsEngine::Shutdown()
    {
        physicsSystem.reset();
        jobSystem.reset();
        tempAllocator.reset();

        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    void JoltPhysicsEngine::Update(float deltaTime)
    {
        const int collisionSteps = 1;
        physicsSystem->Update(deltaTime, collisionSteps, tempAllocator.get(), jobSystem.get());
    }

    void JoltPhysicsEngine::SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation)
    {
        JPH::BodyID joltId(bodyId);
        JPH::Vec3 jPos(position.x, position.y, position.z);
        JPH::Quat jRot(rotation.x, rotation.y, rotation.z, rotation.w);

        physicsSystem->GetBodyInterface().SetPositionAndRotation(joltId, jPos, jRot, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const
    {
        JPH::BodyID joltId(bodyId);
        JPH::Vec3 jPos;
        JPH::Quat jRot;

        physicsSystem->GetBodyInterface().GetPositionAndRotation(joltId, jPos, jRot);

        outPosition = glm::vec3(jPos.GetX(), jPos.GetY(), jPos.GetZ());
        outRotation = glm::quat(jRot.GetW(), jRot.GetX(), jRot.GetY(), jRot.GetZ());
    }

    bool JoltPhysicsEngine::IsBodyActive(PhysicsBodyID bodyId) const
    {
        return physicsSystem->GetBodyInterface().IsActive(JPH::BodyID(bodyId));
    }

    PhysicsBodyID JoltPhysicsEngine::CreateBodyFromShape(JPH::ShapeRefC shape, const glm::vec3& position, const glm::quat& rotation, const PhysicsBodySettings& settings)
    {
        if (!shape) return INVALID_BODY_ID;

        JPH::Vec3 jPos(position.x, position.y, position.z);
        JPH::Quat jRot(rotation.x, rotation.y, rotation.z, rotation.w);
        JPH::EMotionType jMotion = GetJoltMotionType(settings.motionType);
        JPH::ObjectLayer jLayer = static_cast<JPH::ObjectLayer>(settings.layer);

        JPH::BodyCreationSettings joltSettings(shape, jPos, jRot, jMotion, jLayer);
        joltSettings.mFriction = settings.friction;
        joltSettings.mRestitution = settings.restitution;
        joltSettings.mIsSensor = settings.isSensor;

        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
        JPH::Body* body = bodyInterface.CreateBody(joltSettings);

        if (body) {
            JPH::EActivation activation = (settings.motionType == PhysicsMotionType::Static) ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
            bodyInterface.AddBody(body->GetID(), activation);
            return body->GetID().GetIndexAndSequenceNumber();
        }

        return INVALID_BODY_ID;
    }

    PhysicsBodyID JoltPhysicsEngine::CreateBoxBody(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& halfExtents, const PhysicsBodySettings& settings)
    {
        JPH::BoxShapeSettings shapeSettings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
        JPH::ShapeRefC shape = shapeSettings.Create().Get();
        return CreateBodyFromShape(shape, position, rotation, settings);
    }

    PhysicsBodyID JoltPhysicsEngine::CreateSphereBody(const glm::vec3& position, const glm::quat& rotation, float radius, const PhysicsBodySettings& settings)
    {
        JPH::SphereShapeSettings shapeSettings(radius);
        JPH::ShapeRefC shape = shapeSettings.Create().Get();
        return CreateBodyFromShape(shape, position, rotation, settings);
    }

    PhysicsBodyID JoltPhysicsEngine::CreateCapsuleBody(const glm::vec3& position, const glm::quat& rotation, float halfHeight, float radius, const PhysicsBodySettings& settings)
    {
        JPH::CapsuleShapeSettings shapeSettings(halfHeight, radius);
        JPH::ShapeRefC shape = shapeSettings.Create().Get();
        return CreateBodyFromShape(shape, position, rotation, settings);
    }

    PhysicsBodyID JoltPhysicsEngine::CreateConvexBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, const glm::vec3& scale, const PhysicsBodySettings& settings)
    {
        JPH::Array<JPH::Vec3> joltVertices;
        joltVertices.reserve(vertices.size());
        for (const auto& v : vertices) {
            joltVertices.push_back(JPH::Vec3(v.x, v.y, v.z));
        }

        JPH::ConvexHullShapeSettings shapeSettings(joltVertices);
        JPH::ShapeRefC finalShape = shapeSettings.Create().Get();

        if (scale != glm::vec3(1.0f, 1.0f, 1.0f)) {
            JPH::ScaledShapeSettings scaledSettings(finalShape, JPH::Vec3(scale.x, scale.y, scale.z));
            finalShape = scaledSettings.Create().Get();
        }

        return CreateBodyFromShape(finalShape, position, rotation, settings);
    }

    PhysicsBodyID JoltPhysicsEngine::CreateMeshBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, std::span<const uint32_t> indices, const glm::vec3& scale, const PhysicsBodySettings& settings)
    {
        JPH::VertexList joltVertices;
        joltVertices.reserve(vertices.size());
        for (const auto& v : vertices) {
            joltVertices.push_back(JPH::Float3(v.x, v.y, v.z));
        }

        JPH::IndexedTriangleList joltTriangles;
        size_t triangleCount = indices.size() / 3;
        joltTriangles.reserve(triangleCount);
        for (size_t i = 0; i < indices.size(); i += 3) {
            joltTriangles.push_back(JPH::IndexedTriangle(indices[i], indices[i + 1], indices[i + 2]));
        }

        JPH::MeshShapeSettings meshSettings(joltVertices, joltTriangles);
        JPH::ShapeRefC finalShape = meshSettings.Create().Get();

        if (scale != glm::vec3(1.0f, 1.0f, 1.0f)) {
            JPH::ScaledShapeSettings scaledSettings(finalShape, JPH::Vec3(scale.x, scale.y, scale.z));
            finalShape = scaledSettings.Create().Get();
        }

        return CreateBodyFromShape(finalShape, position, rotation, settings);
    }

    void JoltPhysicsEngine::DestroyBody(PhysicsBodyID bodyId)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
        JPH::BodyID joltId(bodyId);

        bodyInterface.RemoveBody(joltId);
        bodyInterface.DestroyBody(joltId);
    }

    void JoltPhysicsEngine::SetBoxShape(PhysicsBodyID bodyId, const glm::vec3& newHalfExtents)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::BoxShapeSettings shapeSettings(JPH::Vec3(newHalfExtents.x, newHalfExtents.y, newHalfExtents.z));
        JPH::ShapeRefC newShape = shapeSettings.Create().Get();
        physicsSystem->GetBodyInterface().SetShape(JPH::BodyID(bodyId), newShape, true, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::SetSphereShape(PhysicsBodyID bodyId, float newRadius)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::SphereShapeSettings shapeSettings(newRadius);
        JPH::ShapeRefC newShape = shapeSettings.Create().Get();
        physicsSystem->GetBodyInterface().SetShape(JPH::BodyID(bodyId), newShape, true, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::SetCapsuleShape(PhysicsBodyID bodyId, float newHalfHeight, float newRadius)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::CapsuleShapeSettings shapeSettings(newHalfHeight, newRadius);
        JPH::ShapeRefC newShape = shapeSettings.Create().Get();
        physicsSystem->GetBodyInterface().SetShape(JPH::BodyID(bodyId), newShape, true, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::SetConvexShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, const glm::vec3& scale)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::Array<JPH::Vec3> joltVertices;
        joltVertices.reserve(newVertices.size());
        for (const auto& v : newVertices) {
            joltVertices.push_back(JPH::Vec3(v.x, v.y, v.z));
        }

        JPH::ConvexHullShapeSettings shapeSettings(joltVertices);
        JPH::ShapeRefC newShape = shapeSettings.Create().Get();

        if (scale != glm::vec3(1.0f, 1.0f, 1.0f)) {
            JPH::ScaledShapeSettings scaledSettings(newShape, JPH::Vec3(scale.x, scale.y, scale.z));
            newShape = scaledSettings.Create().Get();
        }

        physicsSystem->GetBodyInterface().SetShape(JPH::BodyID(bodyId), newShape, true, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::SetMeshShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, std::span<const uint32_t> newIndices, const glm::vec3& scale)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::VertexList joltVertices;
        joltVertices.reserve(newVertices.size());
        for (const auto& v : newVertices) {
            joltVertices.push_back(JPH::Float3(v.x, v.y, v.z));
        }

        JPH::IndexedTriangleList joltTriangles;
        size_t triangleCount = newIndices.size() / 3;
        joltTriangles.reserve(triangleCount);
        for (size_t i = 0; i < newIndices.size(); i += 3) {
            joltTriangles.push_back(JPH::IndexedTriangle(newIndices[i], newIndices[i + 1], newIndices[i + 2]));
        }

        JPH::MeshShapeSettings meshSettings(joltVertices, joltTriangles);
        JPH::ShapeRefC newShape = meshSettings.Create().Get();

        if (scale != glm::vec3(1.0f, 1.0f, 1.0f)) {
            JPH::ScaledShapeSettings scaledSettings(newShape, JPH::Vec3(scale.x, scale.y, scale.z));
            newShape = scaledSettings.Create().Get();
        }

        physicsSystem->GetBodyInterface().SetShape(JPH::BodyID(bodyId), newShape, true, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::SetBodyFriction(PhysicsBodyID bodyId, float friction)
    {
        if (bodyId == INVALID_BODY_ID) return;

        physicsSystem->GetBodyInterface().SetFriction(JPH::BodyID(bodyId), friction);
    }

    void JoltPhysicsEngine::SetBodyRestitution(PhysicsBodyID bodyId, float restitution)
    {
        if (bodyId == INVALID_BODY_ID) return;

        physicsSystem->GetBodyInterface().SetRestitution(JPH::BodyID(bodyId), restitution);
    }

    void JoltPhysicsEngine::SetBodyMotionType(PhysicsBodyID bodyId, PhysicsMotionType motionType)
    {
        if (bodyId == INVALID_BODY_ID) return;

        JPH::EMotionType jMotion = GetJoltMotionType(motionType);
        JPH::EActivation activation = (motionType == PhysicsMotionType::Static) ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;

        physicsSystem->GetBodyInterface().SetMotionType(JPH::BodyID(bodyId), jMotion, activation);
    }

    void JoltPhysicsEngine::GetDebugDrawData(PhysicsDrawData& outData)
    {
        if (!physicsSystem) return;

        JoltLineExtractor extractor;

        JPH::BodyManager::DrawSettings settings;
        settings.mDrawShape = true;
        settings.mDrawShapeWireframe = true;
        settings.mDrawBoundingBox = false;
        settings.mDrawCenterOfMassTransform = false;

        physicsSystem->DrawBodies(settings, &extractor);
        physicsSystem->DrawConstraints(&extractor);

        extractor.BuildDrawData(outData);
    }
}