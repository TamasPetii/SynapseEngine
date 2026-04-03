#include "AnimationBakeProcessor.h"
#include "Engine/ServiceLocator.h"

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Syn
{
    void AnimationBakeProcessor::Process(CookedAnimation& inOutAnimation, const CookedModel& baseModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each_index(0u, inOutAnimation.frameCount, 1u,
            [&](uint32_t frameIndex) {
                BakeFrame(frameIndex, inOutAnimation);
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
    }

    void AnimationBakeProcessor::BakeFrame(uint32_t frameIndex, CookedAnimation& anim)
    {
        float timeInSeconds = static_cast<float>(frameIndex) / anim.sampleRate;
        float timeInTicks = timeInSeconds * anim.ticksPerSecond;

        float animDurationTicks = anim.durationInSeconds * anim.ticksPerSecond;
        if (timeInTicks > animDurationTicks) {
            timeInTicks = animDurationTicks;
        }

        std::vector<glm::mat4> globalTransforms(anim.nodeCount, glm::mat4(1.0f));
        CookedAnimationFrame& currentFrame = anim.frames[frameIndex];

        for (uint32_t i = 0; i < anim.nodeCount; ++i)
        {
            const AnimationNode& node = anim.nodes[i];
            glm::mat4 localTransform = node.defaultLocalTransform;

            if (node.trackIndex != UINT32_MAX)
            {
                const BoneTrack& track = anim.tracks[node.trackIndex];

                glm::vec3 pos = InterpolatePosition(track, timeInTicks);
                glm::quat rot = InterpolateRotation(track, timeInTicks);
                glm::vec3 scale = InterpolateScale(track, timeInTicks);

                localTransform = glm::translate(glm::mat4(1.0f), pos);
                localTransform *= glm::toMat4(glm::normalize(rot));
                localTransform = glm::scale(localTransform, scale);
            }

            if (node.parentIndex == UINT32_MAX)
            {
                globalTransforms[i] = localTransform;
            }
            else
            {
                globalTransforms[i] = globalTransforms[node.parentIndex] * localTransform;
            }

            glm::mat4 globalBoneMatrix = globalTransforms[i] * node.offsetMatrix;
            currentFrame.bakedNodeTransforms[i].localTransform = localTransform;
            currentFrame.bakedNodeTransforms[i].globalTransform = globalBoneMatrix;
            currentFrame.bakedNodeTransforms[i].globalTransformIT = glm::transpose(glm::inverse(globalBoneMatrix));
        }
    }

    glm::vec3 AnimationBakeProcessor::InterpolatePosition(const BoneTrack& track, float time)
    {
        if (track.positions.empty()) return glm::vec3(0.0f);
        if (track.positions.size() == 1 || time <= track.positions.front().time) return track.positions.front().value;
        if (time >= track.positions.back().time) return track.positions.back().value;

        for (size_t i = 0; i < track.positions.size() - 1; ++i)
        {
            if (time < track.positions[i + 1].time)
            {
                float t1 = track.positions[i].time;
                float t2 = track.positions[i + 1].time;
                float factor = (time - t1) / (t2 - t1);
                return glm::mix(track.positions[i].value, track.positions[i + 1].value, factor);
            }
        }
        return track.positions.back().value;
    }

    glm::quat AnimationBakeProcessor::InterpolateRotation(const BoneTrack& track, float time)
    {
        if (track.rotations.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        if (track.rotations.size() == 1 || time <= track.rotations.front().time) return track.rotations.front().value;
        if (time >= track.rotations.back().time) return track.rotations.back().value;

        for (size_t i = 0; i < track.rotations.size() - 1; ++i)
        {
            if (time < track.rotations[i + 1].time)
            {
                float t1 = track.rotations[i].time;
                float t2 = track.rotations[i + 1].time;
                float factor = (time - t1) / (t2 - t1);
                return glm::slerp(track.rotations[i].value, track.rotations[i + 1].value, factor);
            }
        }
        return track.rotations.back().value;
    }

    glm::vec3 AnimationBakeProcessor::InterpolateScale(const BoneTrack& track, float time)
    {
        if (track.scales.empty()) return glm::vec3(1.0f);
        if (track.scales.size() == 1 || time <= track.scales.front().time) return track.scales.front().value;
        if (time >= track.scales.back().time) return track.scales.back().value;

        for (size_t i = 0; i < track.scales.size() - 1; ++i)
        {
            if (time < track.scales[i + 1].time)
            {
                float t1 = track.scales[i].time;
                float t2 = track.scales[i + 1].time;
                float factor = (time - t1) / (t2 - t1);
                return glm::mix(track.scales[i].value, track.scales[i + 1].value, factor);
            }
        }
        return track.scales.back().value;
    }
}