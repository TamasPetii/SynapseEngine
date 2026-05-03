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

### Meshlet Pipeline with LOD (NVIDIA Bistro)

<video controls src="Paper/bistro_meshlet_demo.mp4"></video>

---

### Hierarchical Culling

![Hierarchical Culling](Paper/hierarchical_culling.png)

### Meshlet Visualization

![Meshlet Visualization](Paper/meshlet_visualization.png)

### High Density Scene (1M+ Entities)

![High Density Scene](Paper/high_density_scene.png)

### Debug View / Culling Stages

![Culling Debug](Paper/culling_debug.png)

### LOD / Pipeline Distribution

![LOD Visualization](Paper/lod_visualization.png)

## Research Paper

The architecture and performance characteristics of the engine are described in detail in the accompanying paper:

High-Performance GPU-Driven Rendering and Hierarchical Culling Architecture

Read the paper:
Paper/synapse_engine_paper.pdf

### Summary

* Fully GPU-driven rendering pipeline with minimal CPU involvement
* Multi-stage hierarchical visibility system
* Data-oriented ECS with segmented storage
* Efficient GPU memory layout and indirect draw architecture
* Real-time performance with millions of entities

## Presentations

This project has also been presented in multiple internal and academic contexts.

Note: These presentations reflect earlier iterations of the architecture.

* Paper/presentation_1.pptx
* Paper/presentation_2.pptx

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

## License

This project is licensed under the Apache License 2.0.
See the LICENSE file for details.
