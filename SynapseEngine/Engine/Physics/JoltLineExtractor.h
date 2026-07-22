#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include "Engine/Physics/IPhysicsEngine.h"
#include <memory>
#include <span>
#include <vector>
#include <unordered_map>
#include <atomic>

namespace Syn
{
    class JoltBatchImpl : public JPH::RefTargetVirtual
    {
    public:
        std::vector<glm::vec4> mVertices;
        std::vector<uint32_t> mLineIndices;

        virtual void AddRef() override { ++mRefCount; }
        virtual void Release() override { if (--mRefCount == 0) delete this; }

    private:
        std::atomic<uint32_t> mRefCount{ 0 };
    };

    class JoltLineExtractor final : public JPH::DebugRenderer
    {
    public:
        std::unordered_map<JoltBatchImpl*, std::vector<PhysicsDebugInstance>> mBatchedInstances;
        std::vector<std::pair<glm::vec3, glm::vec3>> mStandaloneLines;
        std::vector<uint32_t> mStandaloneColors;

        JoltLineExtractor() { Initialize(); }

        void DrawLine(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::ColorArg inColor) override
        {
            mStandaloneLines.push_back({
                glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()),
                glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ())
                });
            mStandaloneColors.push_back(inColor.GetUInt32());
        }

        void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override
        {
            DrawLine(inV1, inV2, inColor);
            DrawLine(inV2, inV3, inColor);
            DrawLine(inV3, inV1, inColor);
        }

        Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override
        {
            JoltBatchImpl* batch = new JoltBatchImpl();
            batch->mVertices.reserve(inTriangleCount * 3);
            batch->mLineIndices.reserve(inTriangleCount * 6);

            uint32_t index = 0;
            for (int i = 0; i < inTriangleCount; ++i) {
                for (int v = 0; v < 3; ++v) {
                    batch->mVertices.push_back(glm::vec4(inTriangles[i].mV[v].mPosition.x, inTriangles[i].mV[v].mPosition.y, inTriangles[i].mV[v].mPosition.z, 1.0f));
                }
                batch->mLineIndices.push_back(index + 0); batch->mLineIndices.push_back(index + 1);
                batch->mLineIndices.push_back(index + 1); batch->mLineIndices.push_back(index + 2);
                batch->mLineIndices.push_back(index + 2); batch->mLineIndices.push_back(index + 0);
                index += 3;
            }
            return batch;
        }

        Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override
        {
            JoltBatchImpl* batch = new JoltBatchImpl();
            batch->mVertices.reserve(inVertexCount);
            for (int i = 0; i < inVertexCount; ++i) {
                batch->mVertices.push_back(glm::vec4(inVertices[i].mPosition.x, inVertices[i].mPosition.y, inVertices[i].mPosition.z, 1.0f));
            }

            batch->mLineIndices.reserve((inIndexCount / 3) * 6);
            for (int i = 0; i < inIndexCount; i += 3) {
                uint32_t i0 = inIndices[i]; uint32_t i1 = inIndices[i + 1]; uint32_t i2 = inIndices[i + 2];
                batch->mLineIndices.push_back(i0); batch->mLineIndices.push_back(i1);
                batch->mLineIndices.push_back(i1); batch->mLineIndices.push_back(i2);
                batch->mLineIndices.push_back(i2); batch->mLineIndices.push_back(i0);
            }
            return batch;
        }

        void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override
        {
            if (!inGeometry || inGeometry->mLODs.empty()) return;
            JoltBatchImpl* batch = static_cast<JoltBatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());
            if (!batch) return;

            PhysicsDebugInstance inst;
            inModelMatrix.StoreFloat4x4(reinterpret_cast<JPH::Float4*>(&inst.transform));
            inst.color = inModelColor.GetUInt32();

            mBatchedInstances[batch].push_back(inst);
        }

        void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override {}

        void BuildDrawData(PhysicsDrawData& outData)
        {
            uint32_t baseVertex = 0;
            uint32_t baseIndex = 0;
            uint32_t baseInstance = 0;

            for (const auto& [batch, instances] : mBatchedInstances) {
                for (const auto& v : batch->mVertices) outData.vertices.push_back({ v });
                for (uint32_t idx : batch->mLineIndices) outData.indices.push_back(baseVertex + idx);

                outData.instances.insert(outData.instances.end(), instances.begin(), instances.end());

                VkDrawIndirectCommand cmd{};
                cmd.vertexCount = batch->mLineIndices.size();
                cmd.instanceCount = instances.size();
                cmd.firstVertex = baseIndex;
                cmd.firstInstance = baseInstance;
                outData.indirectCmds.push_back(cmd);

                baseVertex += batch->mVertices.size();
                baseIndex += batch->mLineIndices.size();
                baseInstance += instances.size();
            }

            if (!mStandaloneLines.empty()) {
                uint32_t numLines = mStandaloneLines.size();
                for (size_t i = 0; i < numLines; ++i) {
                    outData.vertices.push_back({ glm::vec4(mStandaloneLines[i].first, 1.0f) });
                    outData.vertices.push_back({ glm::vec4(mStandaloneLines[i].second, 1.0f) });
                    outData.indices.push_back(baseVertex);
                    outData.indices.push_back(baseVertex + 1);

                    PhysicsDebugInstance inst;
                    inst.transform = glm::mat4(1.0f);
                    inst.color = mStandaloneColors[i];
                    outData.instances.push_back(inst);

                    VkDrawIndirectCommand cmd{};
                    cmd.vertexCount = 2;
                    cmd.instanceCount = 1;
                    cmd.firstVertex = baseIndex;
                    cmd.firstInstance = baseInstance;
                    outData.indirectCmds.push_back(cmd);

                    baseVertex += 2;
                    baseIndex += 2;
                    baseInstance += 1;
                }
            }
        }
    };
}