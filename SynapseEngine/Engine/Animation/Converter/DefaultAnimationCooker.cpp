#include "DefaultAnimationCooker.h"
#include "Engine/Mesh/Data/Common/TransformNode.h"
#include <cmath>

namespace Syn
{
    CookedAnimation DefaultAnimationCooker::Cook(const RawAnimation& rawAnimation, float sampleRate)
    {
        CookedAnimation cookedAnim;

        cookedAnim.name = rawAnimation.name;
        cookedAnim.ticksPerSecond = rawAnimation.ticksPerSecond > 0.0f ? rawAnimation.ticksPerSecond : 60.0f;
        cookedAnim.durationInSeconds = rawAnimation.duration / cookedAnim.ticksPerSecond;
        cookedAnim.sampleRate = sampleRate;

        cookedAnim.frameCount = static_cast<uint32_t>(std::ceil(cookedAnim.durationInSeconds * sampleRate)) + 1;
        cookedAnim.nodeCount = static_cast<uint32_t>(rawAnimation.nodes.size());

        cookedAnim.tracks = rawAnimation.tracks;
        cookedAnim.nodes = rawAnimation.nodes;

        cookedAnim.meshSkins.reserve(rawAnimation.meshSkins.size());

        for (const auto& rawSkin : rawAnimation.meshSkins)
        {
            CookedAnimationMeshSkin cookedSkin;
            cookedSkin.vertices = rawSkin.vertices;
            cookedAnim.meshSkins.push_back(std::move(cookedSkin));
        }

        cookedAnim.frames.resize(cookedAnim.frameCount);
        for (auto& frame : cookedAnim.frames)
        {
            frame.bakedNodeTransforms.resize(cookedAnim.nodeCount);
        }

        return cookedAnim;
    }
}