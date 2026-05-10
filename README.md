# Synapse Engine

High-performance GPU-driven rendering engine built around a fully data-oriented architecture, designed to scale to millions of entities in real-time.

## Overview

Synapse Engine is a research-oriented real-time rendering engine focusing on eliminating CPU bottlenecks and maximizing GPU utilization through a fully compute-driven pipeline.

The system integrates a segmented data-oriented ECS with a hierarchical GPU culling architecture and modern mesh shader support.

## Core Concepts

* GPU-driven rendering (indirect draw, compute-based visibility)
* Hierarchical culling (Model → Mesh → Meshlet)
* Segmented ECS (Static / Dynamic / Stream)
* Sparse-set O(1) component access
* Bindless resource management
* Mesh shader pipeline integration

## Visual Demonstration

### High Density Scene (1M+ Entities)

The architecture is built to handle extreme entity counts without CPU bottlenecks, leveraging the segmented ECS and indirect draw calls.

**Exterior Overview:** 
A wide shot demonstrating over a million uniquely colored and scaled primitives rendered in real-time.
![High Density Scene - Exterior](Docs/Images/IntroScene.png)

**Interior Perspective:** 
Navigating through dense geometry clusters within an architectural environment.
![High Density Scene - Interior](Docs/Images/Bloom.png)

**Performance Demonstration:** 
Watch the engine fluidly handle 1,000,000 static objects and animations dynamic objects while maintaining exceptionally low frame times.

[![Synapse Engine - High Density Scene Rendering](https://img.youtube.com/vi/btftJGd3JzA/maxresdefault.jpg)](https://www.youtube.com/watch?v=btftJGd3JzA)
*(Click the image to watch the full performance test on YouTube)*

### Meshlet Pipeline with LOD (NVIDIA Bistro)

This video demonstrates the real-time generation and transition of meshlets in the highly detailed NVIDIA Bistro environment, showcasing dynamic level-of-detail scaling directly on the GPU.

[![Synapse Engine - Mesh Shader Pipeline & LOD in NVIDIA Bistro](https://img.youtube.com/vi/h64PygG19x4/maxresdefault.jpg)](https://www.youtube.com/watch?v=h64PygG19x4)
*(Click the image to watch the meshlet demonstration on YouTube)*

### Hierarchical Culling

The engine utilizes a multi-stage culling approach to ensure minimal waste of GPU resources.

**Frustum Culling:** 
Geometry completely outside the camera's view is discarded before reaching the rasterizer.
![Frustum Culling](Docs/Images/Frustum.png)

**Occlusion Culling:** 
Objects hidden behind other opaque structures are efficiently culled using Hi-Z occlusion tests, drastically reducing overdraw.
![Occlusion Culling](Docs/Images/Occlusion.png)

### Scene Composition & Culling Debug (Dragon View)

This scene highlights the integration of complex, high-poly geometry (such as the Stanford Dragon) alongside thousands of dynamic, emissive primitives. It demonstrates the flexibility of the bindless resource system handling various mesh types simultaneously.

![Scene Composition with Dragon](Docs/Images/CullingBefore.png)
![Meshlet Visualization](Docs/Images/CullingAfter.png)

## Research Paper

The architecture and performance characteristics of the engine are described in detail in the accompanying paper:

High-Performance GPU-Driven Rendering and Hierarchical Culling Architecture

[**Read the full Research Paper (PDF)**](Docs/Papers/Grafgeo_High_Performance_Gpu_Driven_Rendering.pdf)

### Summary

* Fully GPU-driven rendering pipeline with minimal CPU involvement
* Multi-stage hierarchical visibility system
* Data-oriented ECS with segmented storage
* Efficient GPU memory layout and indirect draw architecture
* Real-time performance with millions of entities

## Presentations

This project has also been presented in multiple internal and academic contexts.

*Note: These presentations reflect earlier iterations of the architecture.*

* [**Synapse Engine v1.0**](Docs/Presentations/SynapseEngine_v1.0.pdf)
* [**Synapse Engine v1.1**](Docs/Presentations/SynapseEngine_v1.1.pdf)

## Architecture Highlights

### GPU-Driven Pipeline

The engine removes per-object CPU draw submission entirely. Visibility determination, instance selection, and draw command generation are executed on the GPU using compute shaders and indirect draw buffers.

### Hierarchical Culling

Visibility is resolved across three levels:

* Model-level (coarse filtering)
* Mesh-level (collaborative compute pass)
* Meshlet-level (task shader culling)

Techniques used:

* Frustum culling
* Hi-Z occlusion
* Cone culling
* Zero-pixel triangle rejection

### Segmented ECS

The ECS is structured into three regions:

* Static: rarely changing data
* Dynamic: moderately changing data with change tracking
* Stream: per-frame updated data

This enables:

* minimal iteration overhead
* efficient parallel processing
* reduced CPU-GPU synchronization

### Bindless Resource System

All models, materials, and animations are accessed via index-based indirection, enabling fully GPU-resolved resource access without traditional binding overhead.

## Performance

* Handles 1,000,000+ entities in real-time
* GPU-driven culling reduces CPU cost to near-zero
* Mesh shader pipeline achieves up to ~2x speedup compared to traditional pipelines
* Near-linear scaling across modern GPU architectures

## Build

1. Clone the repository with submodules:

```
git clone --recursive <repo_url>
```

If already cloned:

```
git submodule update --init --recursive
```

2. Bootstrap vcpkg:

```
bootstrap-vcpkg.bat
```

3. Open the solution in Visual Studio 2026 and build.

If vcpkg is integrated with Visual Studio, all dependencies will be automatically resolved and installed during the build process (manifest mode).

## Notes

* This is a research and architecture-focused engine
* Not intended as a plug-and-play game engine
* Designed to explore modern high-performance rendering techniques

## Licensing

SynapseEngine is dual-licensed:

- GNU Affero General Public License v3.0 (AGPLv3) for open-source and non-commercial use
- Commercial license available for proprietary/commercial usage

If you want to use SynapseEngine in a closed-source product,
commercial game,
proprietary engine,
or commercial environment without AGPL obligations,
you must obtain a commercial license.

For commercial licensing inquiries:

tamaspeti3451@gmail.com
