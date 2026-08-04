#include "Box3DPhysicsEngine.h"
#include "Engine/ServiceLocator.h"
#include <taskflow/taskflow.hpp>

namespace Syn
{
    struct Box3DTaskWrapper {
        tf::Taskflow taskflow;
        std::future<void> future;
    };

    static b3BodyType GetBox3DMotionType(PhysicsMotionType type)
    {
        switch (type)
        {
        case PhysicsMotionType::Static:    return b3_staticBody;
        case PhysicsMotionType::Kinematic: return b3_kinematicBody;
        case PhysicsMotionType::Dynamic:   return b3_dynamicBody;
        default:                           return b3_dynamicBody;
        }
    }

    b3Quat Box3DPhysicsEngine::GlmToB3Quat(const glm::quat& q) const
    {
        b3Quat b3q;
        b3q.v.x = q.x;
        b3q.v.y = q.y;
        b3q.v.z = q.z;
        b3q.s = q.w;
        return b3q;
    }

    Box3DPhysicsEngine::Box3DPhysicsEngine()
    {}

    Box3DPhysicsEngine::~Box3DPhysicsEngine()
    {
        Shutdown();
    }

    void Box3DPhysicsEngine::Init(const PhysicsInitParams& params)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3WorldDef worldDef = b3DefaultWorldDef();

        tf::Executor* executor = ServiceLocator::Get<tf::Executor>();
        worldDef.userTaskContext = executor;
        worldDef.workerCount = executor->num_workers();

        worldDef.enqueueTask = [](b3TaskCallback* task, void* taskContext, void* userContext, const char* taskName) -> void* {
            tf::Executor* exec = static_cast<tf::Executor*>(userContext);
            Box3DTaskWrapper* wrapper = new Box3DTaskWrapper();

            wrapper->taskflow.emplace([task, taskContext]() {
                task(taskContext);
                }).name(taskName ? taskName : "Box3DTask");

            wrapper->future = exec->run(wrapper->taskflow);

            return wrapper;
            };

        worldDef.finishTask = [](void* userTask, void* userContext) {
            if (userTask) {
                Box3DTaskWrapper* wrapper = static_cast<Box3DTaskWrapper*>(userTask);
                wrapper->future.wait();
                delete wrapper;
            }
            };

        _worldId = b3CreateWorld(&worldDef);
    }

    void Box3DPhysicsEngine::Shutdown()
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        if (b3World_IsValid(_worldId)) {
            b3DestroyWorld(_worldId);
        }

        for (auto& entity : _bodyMap) {
            if (entity.meshData) b3DestroyMesh(entity.meshData);
        }

        _bodyMap.clear();
        _freeSlots.clear();
    }

    void Box3DPhysicsEngine::Update(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        const int32_t subStepCount = 4;
        b3World_Step(_worldId, deltaTime, subStepCount);
    }

    // Privát metódus: Nem igényel saját lockot, feltételezi, hogy a hívó már levédte
    PhysicsBodyID Box3DPhysicsEngine::AddEntity(b3BodyId b3Id, b3ShapeId b3Shape, const PhysicsBodySettings& settings, b3MeshData* meshData)
    {
        B3Entity entity{ b3Id, b3Shape, meshData, settings };
        if (!_freeSlots.empty()) {
            PhysicsBodyID id = _freeSlots.back();
            _freeSlots.pop_back();
            _bodyMap[id] = entity;
            return id;
        }
        _bodyMap.push_back(entity);
        return static_cast<PhysicsBodyID>(_bodyMap.size() - 1);
    }

    // Privát metódus: Nem igényel saját lockot
    Box3DPhysicsEngine::B3Entity Box3DPhysicsEngine::GetEntity(PhysicsBodyID id) const
    {
        if (id < _bodyMap.size()) {
            return _bodyMap[id];
        }
        return { b3_nullBodyId, b3_nullShapeId, nullptr, {} };
    }

    b3ShapeDef Box3DPhysicsEngine::CreateShapeDef(const PhysicsBodySettings& settings) const
    {
        b3ShapeDef shapeDef = b3DefaultShapeDef();
        shapeDef.density = settings.mass;
        shapeDef.baseMaterial.friction = settings.friction;
        shapeDef.baseMaterial.restitution = settings.restitution;
        shapeDef.isSensor = settings.isSensor;
        shapeDef.filter.categoryBits = 1 << settings.layer;
        return shapeDef;
    }

    // Privát metódus: Nem igényel saját lockot
    void Box3DPhysicsEngine::ReplaceShape(PhysicsBodyID bodyId, b3ShapeId newShape, b3MeshData* newMeshData)
    {
        B3Entity& entity = _bodyMap[bodyId];
        if (b3Shape_IsValid(entity.shape)) {
            b3DestroyShape(entity.shape, true);
        }
        if (entity.meshData) {
            b3DestroyMesh(entity.meshData);
        }
        entity.shape = newShape;
        entity.meshData = newMeshData;
    }

    void Box3DPhysicsEngine::SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyId b3Id = GetEntity(bodyId).body;
        if (b3Body_IsValid(b3Id)) {
            b3Body_SetTransform(b3Id, { position.x, position.y, position.z }, GlmToB3Quat(rotation));
            b3Body_SetAwake(b3Id, true);
        }
    }

    void Box3DPhysicsEngine::GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyId b3Id = GetEntity(bodyId).body;
        if (b3Body_IsValid(b3Id)) {
            b3Vec3 pos = b3Body_GetPosition(b3Id);
            b3Quat rot = b3Body_GetRotation(b3Id);
            outPosition = glm::vec3(pos.x, pos.y, pos.z);
            outRotation = glm::quat(rot.s, rot.v.x, rot.v.y, rot.v.z);
        }
    }

    bool Box3DPhysicsEngine::IsBodyActive(PhysicsBodyID bodyId) const
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyId b3Id = GetEntity(bodyId).body;
        return b3Body_IsValid(b3Id) ? b3Body_IsAwake(b3Id) : false;
    }

    PhysicsBodyID Box3DPhysicsEngine::CreateBoxBody(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& halfExtents, const PhysicsBodySettings& settings)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = GetBox3DMotionType(settings.motionType);
        bodyDef.position = { position.x, position.y, position.z };
        bodyDef.rotation = GlmToB3Quat(rotation);
        b3BodyId b3Id = b3CreateBody(_worldId, &bodyDef);

        b3BoxHull boxHull = b3MakeBoxHull(halfExtents.x, halfExtents.y, halfExtents.z);
        b3ShapeDef shapeDef = CreateShapeDef(settings);
        b3ShapeId shapeId = b3CreateHullShape(b3Id, &shapeDef, &boxHull.base);

        return AddEntity(b3Id, shapeId, settings);
    }

    PhysicsBodyID Box3DPhysicsEngine::CreateSphereBody(const glm::vec3& position, const glm::quat& rotation, float radius, const PhysicsBodySettings& settings)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = GetBox3DMotionType(settings.motionType);
        bodyDef.position = { position.x, position.y, position.z };
        bodyDef.rotation = GlmToB3Quat(rotation);
        b3BodyId b3Id = b3CreateBody(_worldId, &bodyDef);

        b3Sphere sphere = { {0.0f, 0.0f, 0.0f}, radius };
        b3ShapeDef shapeDef = CreateShapeDef(settings);
        b3ShapeId shapeId = b3CreateSphereShape(b3Id, &shapeDef, &sphere);

        return AddEntity(b3Id, shapeId, settings);
    }

    PhysicsBodyID Box3DPhysicsEngine::CreateCapsuleBody(const glm::vec3& position, const glm::quat& rotation, float halfHeight, float radius, const PhysicsBodySettings& settings)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = GetBox3DMotionType(settings.motionType);
        bodyDef.position = { position.x, position.y, position.z };
        bodyDef.rotation = GlmToB3Quat(rotation);
        b3BodyId b3Id = b3CreateBody(_worldId, &bodyDef);

        b3Capsule capsule = { {0.0f, -halfHeight, 0.0f}, {0.0f, halfHeight, 0.0f}, radius };
        b3ShapeDef shapeDef = CreateShapeDef(settings);
        b3ShapeId shapeId = b3CreateCapsuleShape(b3Id, &shapeDef, &capsule);

        return AddEntity(b3Id, shapeId, settings);
    }

    PhysicsBodyID Box3DPhysicsEngine::CreateConvexBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, const glm::vec3& scale, const PhysicsBodySettings& settings)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = GetBox3DMotionType(settings.motionType);
        bodyDef.position = { position.x, position.y, position.z };
        bodyDef.rotation = GlmToB3Quat(rotation);
        b3BodyId b3Id = b3CreateBody(_worldId, &bodyDef);

        std::vector<b3Vec3> b3Verts(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            b3Verts[i] = { vertices[i].x * scale.x, vertices[i].y * scale.y, vertices[i].z * scale.z };
        }

        b3HullData* hull = b3CreateHull(b3Verts.data(), static_cast<int>(b3Verts.size()), static_cast<int>(b3Verts.size()));
        b3ShapeDef shapeDef = CreateShapeDef(settings);
        b3ShapeId shapeId = b3_nullShapeId;

        if (hull) {
            shapeId = b3CreateHullShape(b3Id, &shapeDef, hull);
            b3DestroyHull(hull);
        }

        return AddEntity(b3Id, shapeId, settings);
    }

    PhysicsBodyID Box3DPhysicsEngine::CreateMeshBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, std::span<const uint32_t> indices, const glm::vec3& scale, const PhysicsBodySettings& settings)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        b3BodyDef bodyDef = b3DefaultBodyDef();
        bodyDef.type = GetBox3DMotionType(settings.motionType);
        bodyDef.position = { position.x, position.y, position.z };
        bodyDef.rotation = GlmToB3Quat(rotation);
        b3BodyId b3Id = b3CreateBody(_worldId, &bodyDef);

        std::vector<b3Vec3> b3Verts(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            b3Verts[i] = { vertices[i].x, vertices[i].y, vertices[i].z };
        }

        b3MeshDef meshDef = { 0 };
        meshDef.vertices = b3Verts.data();
        meshDef.vertexCount = static_cast<int>(b3Verts.size());
        meshDef.indices = reinterpret_cast<int32_t*>(const_cast<uint32_t*>(indices.data()));
        meshDef.triangleCount = static_cast<int>(indices.size() / 3);

        b3MeshData* meshData = b3CreateMesh(&meshDef, nullptr, 0);

        b3ShapeDef shapeDef = CreateShapeDef(settings);
        b3ShapeId shapeId = b3CreateMeshShape(b3Id, &shapeDef, meshData, { scale.x, scale.y, scale.z });

        return AddEntity(b3Id, shapeId, settings, meshData);
    }

    void Box3DPhysicsEngine::DestroyBody(PhysicsBodyID bodyId)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            b3DestroyBody(entity.body);

            if (entity.meshData) {
                b3DestroyMesh(entity.meshData);
            }

            _bodyMap[bodyId] = { b3_nullBodyId, b3_nullShapeId, nullptr, {} };
            _freeSlots.push_back(bodyId);
        }
    }

    void Box3DPhysicsEngine::SetBoxShape(PhysicsBodyID bodyId, const glm::vec3& newHalfExtents)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            b3ShapeDef shapeDef = CreateShapeDef(entity.settings);
            b3BoxHull boxHull = b3MakeBoxHull(newHalfExtents.x, newHalfExtents.y, newHalfExtents.z);
            b3ShapeId newShape = b3CreateHullShape(entity.body, &shapeDef, &boxHull.base);
            ReplaceShape(bodyId, newShape);
        }
    }

    void Box3DPhysicsEngine::SetSphereShape(PhysicsBodyID bodyId, float newRadius)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            b3ShapeDef shapeDef = CreateShapeDef(entity.settings);
            b3Sphere sphere = { {0.0f, 0.0f, 0.0f}, newRadius };
            b3ShapeId newShape = b3CreateSphereShape(entity.body, &shapeDef, &sphere);
            ReplaceShape(bodyId, newShape);
        }
    }

    void Box3DPhysicsEngine::SetCapsuleShape(PhysicsBodyID bodyId, float newHalfHeight, float newRadius)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            b3ShapeDef shapeDef = CreateShapeDef(entity.settings);
            b3Capsule capsule = { {0.0f, -newHalfHeight, 0.0f}, {0.0f, newHalfHeight, 0.0f}, newRadius };
            b3ShapeId newShape = b3CreateCapsuleShape(entity.body, &shapeDef, &capsule);
            ReplaceShape(bodyId, newShape);
        }
    }

    void Box3DPhysicsEngine::SetConvexShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, const glm::vec3& scale)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            std::vector<b3Vec3> b3Verts(newVertices.size());
            for (size_t i = 0; i < newVertices.size(); ++i) {
                b3Verts[i] = { newVertices[i].x * scale.x, newVertices[i].y * scale.y, newVertices[i].z * scale.z };
            }

            b3HullData* hull = b3CreateHull(b3Verts.data(), static_cast<int>(b3Verts.size()), static_cast<int>(b3Verts.size()));
            b3ShapeDef shapeDef = CreateShapeDef(entity.settings);
            b3ShapeId newShape = b3_nullShapeId;

            if (hull) {
                newShape = b3CreateHullShape(entity.body, &shapeDef, hull);
                b3DestroyHull(hull);
            }

            ReplaceShape(bodyId, newShape);
        }
    }

    void Box3DPhysicsEngine::SetMeshShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, std::span<const uint32_t> newIndices, const glm::vec3& scale)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity entity = GetEntity(bodyId);
        if (b3Body_IsValid(entity.body)) {
            std::vector<b3Vec3> b3Verts(newVertices.size());
            for (size_t i = 0; i < newVertices.size(); ++i) {
                b3Verts[i] = { newVertices[i].x, newVertices[i].y, newVertices[i].z };
            }

            b3MeshDef meshDef = { 0 };
            meshDef.vertices = b3Verts.data();
            meshDef.vertexCount = static_cast<int>(b3Verts.size());
            meshDef.indices = reinterpret_cast<int32_t*>(const_cast<uint32_t*>(newIndices.data()));
            meshDef.triangleCount = static_cast<int>(newIndices.size() / 3);

            b3MeshData* newMeshData = b3CreateMesh(&meshDef, nullptr, 0);
            b3ShapeDef shapeDef = CreateShapeDef(entity.settings);
            b3ShapeId newShape = b3CreateMeshShape(entity.body, &shapeDef, newMeshData, { scale.x, scale.y, scale.z });

            ReplaceShape(bodyId, newShape, newMeshData);
        }
    }

    void Box3DPhysicsEngine::SetBodyFriction(PhysicsBodyID bodyId, float friction)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity& entity = _bodyMap[bodyId];
        entity.settings.friction = friction;
        b3ShapeId b3Shape = entity.shape;
        if (b3Shape_IsValid(b3Shape)) {
            b3Shape_SetFriction(b3Shape, friction);
        }
    }

    void Box3DPhysicsEngine::SetBodyRestitution(PhysicsBodyID bodyId, float restitution)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity& entity = _bodyMap[bodyId];
        entity.settings.restitution = restitution;
        b3ShapeId b3Shape = entity.shape;
        if (b3Shape_IsValid(b3Shape)) {
            b3Shape_SetRestitution(b3Shape, restitution);
        }
    }

    void Box3DPhysicsEngine::SetBodyMotionType(PhysicsBodyID bodyId, PhysicsMotionType motionType)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        B3Entity& entity = _bodyMap[bodyId];
        entity.settings.motionType = motionType;
        b3BodyId b3Id = entity.body;
        if (b3Body_IsValid(b3Id)) {
            b3Body_SetType(b3Id, GetBox3DMotionType(motionType));
            b3Body_SetAwake(b3Id, true);
        }
    }

    void Box3DPhysicsEngine::GetDebugDrawData(PhysicsDrawData& outData)
    {
        std::lock_guard<std::mutex> lock(_engineMutex);
        if (!b3World_IsValid(_worldId)) return;

        b3DebugDraw drawCtx = b3DefaultDebugDraw();
        drawCtx.context = &outData;

        drawCtx.DrawSegmentFcn = [](b3Pos p1, b3Pos p2, b3HexColor color, void* context) {
            PhysicsDrawData* data = static_cast<PhysicsDrawData*>(context);
            uint32_t baseVertex = static_cast<uint32_t>(data->vertices.size());

            uint32_t colorU32 = color | 0xFF000000;

            data->vertices.push_back({ glm::vec4(p1.x, p1.y, p1.z, 1.0f) });
            data->vertices.push_back({ glm::vec4(p2.x, p2.y, p2.z, 1.0f) });

            data->indices.push_back(baseVertex);
            data->indices.push_back(baseVertex + 1);

            PhysicsDebugInstance inst;
            inst.transform = glm::mat4(1.0f);
            inst.color = colorU32;
            data->instances.push_back(inst);

            VkDrawIndirectCommand cmd{};
            cmd.vertexCount = 2;
            cmd.instanceCount = 1;
            cmd.firstVertex = static_cast<uint32_t>(data->indices.size() - 2);
            cmd.firstInstance = static_cast<uint32_t>(data->instances.size() - 1);
            data->indirectCmds.push_back(cmd);
            };

        drawCtx.DrawBoundsFcn = [](b3AABB bounds, b3HexColor color, void* context) {
            PhysicsDrawData* data = static_cast<PhysicsDrawData*>(context);
            uint32_t baseVertex = static_cast<uint32_t>(data->vertices.size());
            uint32_t colorU32 = color | 0xFF000000;

            glm::vec3 minP = { bounds.lowerBound.x, bounds.lowerBound.y, bounds.lowerBound.z };
            glm::vec3 maxP = { bounds.upperBound.x, bounds.upperBound.y, bounds.upperBound.z };

            glm::vec3 corners[8] = {
                { minP.x, minP.y, minP.z }, { maxP.x, minP.y, minP.z },
                { maxP.x, maxP.y, minP.z }, { minP.x, maxP.y, minP.z },
                { minP.x, minP.y, maxP.z }, { maxP.x, minP.y, maxP.z },
                { maxP.x, maxP.y, maxP.z }, { minP.x, maxP.y, maxP.z }
            };

            for (int i = 0; i < 8; ++i) {
                data->vertices.push_back({ glm::vec4(corners[i], 1.0f) });
            }

            uint32_t boxIndices[] = {
                0,1, 1,2, 2,3, 3,0,
                4,5, 5,6, 6,7, 7,4,
                0,4, 1,5, 2,6, 3,7
            };

            for (int i = 0; i < 24; ++i) {
                data->indices.push_back(baseVertex + boxIndices[i]);
            }

            PhysicsDebugInstance inst;
            inst.transform = glm::mat4(1.0f);
            inst.color = colorU32;
            data->instances.push_back(inst);

            VkDrawIndirectCommand cmd{};
            cmd.vertexCount = 24;
            cmd.instanceCount = 1;
            cmd.firstVertex = static_cast<uint32_t>(data->indices.size() - 24);
            cmd.firstInstance = static_cast<uint32_t>(data->instances.size() - 1);
            data->indirectCmds.push_back(cmd);
            };

        drawCtx.DrawShapeFcn = [](void* userShape, b3WorldTransform transform, b3HexColor color, void* context) -> bool {
            return true;
            };

        drawCtx.drawShapes = true;
        drawCtx.drawBounds = true;
        drawCtx.drawJoints = true;

        b3World_Draw(_worldId, &drawCtx, UINT64_MAX);
    }
}