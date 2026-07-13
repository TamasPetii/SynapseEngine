# Synapse Engine

High-performance GPU-driven rendering engine built around a fully data-oriented architecture, designed to scale to millions of entities in real-time.

## Overview

Synapse Engine is a research-oriented real-time rendering engine focusing on eliminating CPU bottlenecks and maximizing GPU utilization through a fully compute-driven pipeline. 

The system integrates a segmented data-oriented ECS with a hierarchical GPU culling architecture, modern mesh shader support, and an advanced virtualized shadow map system for directional, spot, and point lights (supporting both CPU and GPU-driven paths). The engine now utilizes an MVI architecture for its tooling and features a robust cross-platform build system using `xmake` for Windows and Linux.

![Synapse Engine Scene Workspace](Docs/Images/SceneWorkspace.png)
*Synapse Engine running with the integrated ImGui editor (Scene Workspace), showcasing the MVI architecture.*

![Synapse Engine Model Workspace](Docs/Images/ModelWorkspace.png)
*Model Workspace*

![Synapse Engine Material Workspace](Docs/Images/MaterialWorkspace.png)
*Material Workspace*

![Synapse Engine Texture Workspace](Docs/Images/TextureWorkspace.png)
*Texture Workspace*

## Core Concepts

* **GPU-driven rendering**: Indirect draw and compute-based visibility.
* **Hierarchical culling in Compute Shaders**: CPU-generated static chunks and GPU-generated Morton chunks, executing a Chunk → Model → Mesh pipeline, followed by Task Shader Meshlet culling.
* **Template-based Custom ECS Registry**: Natively handles static, dynamic, and stream entities per component with O(1) access.
* **Full Bindless Vulkan**: Utilizing a bindless architecture and the most modern Vulkan extensions.
* **Hybrid Pipeline**: Simultaneous support for traditional and modern Mesh Shader rendering paths.
* **Lighting & Materials**: PBR rendering with an advanced, virtualized shadow map simulation.

## Visual Demonstration

### High Density Scene (1M+ Entities)

The architecture is built to handle extreme entity counts without CPU bottlenecks, leveraging the segmented ECS and indirect draw calls.

**Exterior Overview:** A wide shot demonstrating over a million uniquely colored and scaled primitives rendered in real-time.
![High Density Scene - Exterior](Docs/Images/IntroScene.png)

**Interior Perspective:** Navigating through dense geometry clusters within an architectural environment.
![High Density Scene - Interior](Docs/Images/Bloom.png)

**Performance Demonstration:** Watch the engine fluidly handle 1,000,000 static objects and dynamic animations while maintaining exceptionally low frame times.

[![Synapse Engine - High Density Scene Rendering](https://img.youtube.com/vi/btftJGd3JzA/maxresdefault.jpg)](https://www.youtube.com/watch?v=btftJGd3JzA)
*(Click the image to watch the full performance test on YouTube)*

### Meshlet Pipeline with LOD (NVIDIA Bistro)

This video demonstrates the real-time generation and transition of meshlets in the highly detailed NVIDIA Bistro environment, showcasing dynamic level-of-detail scaling directly on the GPU.

[![Synapse Engine - Mesh Shader Pipeline & LOD in NVIDIA Bistro](https://img.youtube.com/vi/h64PygG19x4/maxresdefault.jpg)](https://www.youtube.com/watch?v=h64PygG19x4)
*(Click the image to watch the meshlet demonstration on YouTube)*

## Lighting Architectures

### Forward+ Rendering

The engine implements an advanced Forward+ rendering path featuring dynamic, Hi-Z based variable tile sizing. Tiles are dynamically split into clusters based on depth information to optimize light culling.

![Cluster Visualization Heatmap](Docs/Images/ClusterHeatmap.png)
*Dynamic tile and cluster visualization based on Hi-Z data.*

![Light Cluster Heatmap](Docs/Images/LightClusterHeatmap.png)
*Light cluster heatmap in a highly populated scene.*

### Deferred Rendering

Deferred rendering is fully supported alongside the Forward+ pipeline. Light volumes are accurately represented during the lighting pass.

![Light Rendering](Docs/Images/LightSimulation.png)
*1000+ Light simulation (No Bloom)*

## Virtualized Shadow Mapping

Synapse Engine employs a fully virtualized shadow map architecture backed by a unified 4K texture atlas. Shadows can be toggled between CPU-driven and full GPU-driven pipelines at runtime. All geometries belonging to a specific light are batch-rendered efficiently.

### Directional Lights (Cascades)
Directional light shadow cascades are allocated directly into the main atlas. The pipeline utilizes the full Chunk → Model → Mesh → Meshlet culling hierarchy to only render the necessary shadow casters into the cascade regions.

![Directional Light Atlas](Docs/Images/DirLightAtlas.png)
![Directional Light Simulation](Docs/Images/DirLightShadow.png)

### Point Lights (Omnidirectional)
Omnidirectional shadows are mapped by allocating 3x2 chunks for the cube faces and projecting them flattened into the virtual 4K shadow map. This drastically reduces the overhead of traditional cubemap rendering.

![Point Light Atlas](Docs/Images/PointLightAtlas.png)
![Point Light Simulation](Docs/Images/PointLightShadow.png)

### Spot Lights
Similar to the other light types, each spot light manages its own localized atlas region within the virtualized shadow map.

![Spot Light Atlas](Docs/Images/SpotLightAtlas.png)
![Spot Light Simulation](Docs/Images/SpotLightShadow.png)

## Hierarchical Culling

The engine utilizes a multi-stage culling approach to ensure minimal waste of GPU resources. **The culling pipeline can be dynamically switched between CPU-driven and full GPU-driven modes at runtime.**

### Chunk Culling
The macro-level culling operates on spatial chunks. Static entities are grouped into CPU-generated chunks, while dynamic entities utilize GPU-generated Morton chunks. 

![Chunk Culling Debug](Docs/Images/ChunkDebug.png)
*Visualization of active spatial chunks.*

### Model, Mesh, and Meshlet Culling
Culling is executed in highly parallel compute shaders. Passing entities trigger indirect draw dispatches.
* **Model Culling:** Coarse filtering of entire objects.
* **Mesh Culling:** Finer filtering at the sub-mesh level.
* **Meshlet Culling:** Executed in the Task Shader for the ultimate granular visibility check.

### Frustum, Occlusion & Cone Culling

* **Frustum Culling:** Geometry completely outside the camera's view is discarded before reaching the rasterizer.
![Frustum Culling](Docs/Images/Frustum.png)

* **Occlusion Culling:** Objects hidden behind other opaque structures are efficiently culled using Hi-Z occlusion tests, drastically reducing overdraw.
![Occlusion Culling Rendering](Docs/Images/Occlusion.png)

* **Cone Culling:** Back-facing meshlets and geometry clusters are efficiently rejected early in the pipeline using normal cone tests, further reducing the number of processed primitives.
![Cone Culling](Docs/Images/ConeCulling.png)

### Scene Composition & Culling Debug (Dragon View)

This scene highlights the integration of complex, high-poly geometry alongside thousands of dynamic, emissive primitives. It demonstrates the flexibility of the bindless resource system handling various mesh types simultaneously.

![Scene Composition with Dragon](Docs/Images/CullingBefore.png)
![Meshlet Visualization](Docs/Images/CullingAfter.png)

### Visual Debug Tools

The engine features comprehensive visual debugging capabilities to inspect the internal state of the scene and the rendering pipeline in real-time. This includes the ability to toggle wireframe rendering down to the granular meshlet level, as well as visualizing various bounding volumes and colliders.

![Meshlet Collider Visualization](Docs/Images/MeshletColliders.png)
*Wireframe and bounding volume visualization at the meshlet level.*

![Light Collider Visualization](Docs/Images/LightColliders.png)
*Debug rendering of light colliders and spatial bounds.*

## Research Paper

The architecture and performance characteristics of the engine are described in detail in the accompanying paper:

High-Performance GPU-Driven Rendering and Hierarchical Culling Architecture

[**Read the full Research Paper (PDF)**](Docs/Papers/Grafgeo_High_Performance_Gpu_Driven_Rendering.pdf)

## Presentations

This project has also been presented in multiple internal and academic contexts. 

* [**Synapse Engine v1.0**](Docs/Presentations/SynapseEngine_v1.0.pdf)
* [**Synapse Engine v1.1**](Docs/Presentations/SynapseEngine_v1.1.pdf)
* [**Synapse Engine v1.2**](Docs/Presentations/BEMUTATO_GRAFGEO_TAMAS_PETER.pdf)

## Performance

* Handles 1,000,000+ entities in real-time
* GPU-driven culling reduces CPU cost to near-zero
* Mesh shader pipeline achieves up to ~2x speedup compared to traditional pipelines
* Near-linear scaling across modern GPU architectures

## Build

We use **xmake** for seamless cross-platform building on Windows and Linux.

> Unless stated otherwise, all `xmake` commands should be executed from the `SynapseEngine` directory (the repository root), where the `xmake.lua` file is located.

---

### Windows Setup

#### 1. Install Xmake

You can install **xmake** using PowerShell via **winget** or **Chocolatey**.

```powershell
winget install xmake

# or

choco install xmake
```

#### 2. Clone the Repository

Clone the repository along with its submodules.

```bash
git clone --recursive https://github.com/TamasPetii/SynapseEngine.git
```

If the repository has already been cloned, initialize or update the submodules:

```bash
git submodule update --init --recursive
```

#### 3. Bootstrap vcpkg

```powershell
cd External/vcpkg
.\bootstrap-vcpkg.bat
cd ..\..
```

---

### IDE Integration (Windows)

#### Visual Studio Code

Install the **Xmake** extension for Visual Studio Code.

The extension provides:

- Xmake Explorer
- Target Editor
- Build Mode selector (Debug / Release / Dist / Performance)
- Build & Run buttons

![Xmake Visual Studio Code](Docs/Images/XmakeVsCode.png)

#### Visual Studio 2026 (.sln)

Generate a native Visual Studio solution:

```bash
xmake project -k vsxmake -m "debug,release,dist,performance" .
```

#### CMake + Visual Studio 2026

Generate a `CMakeLists.txt` for traditional CMake workflows:

```bash
xmake project -k cmake -m "debug,release,dist,performance" .
```

---

### Linux Setup (Ubuntu)

> **Important**
>
> SynapseEngine requires **GCC 14**. Both **vcpkg** and the engine **must** be built using the exact same compiler (`gcc-14` / `g++-14`) to guarantee ABI compatibility and consistent standard library behavior.

#### 1. Install Dependencies

```bash
sudo apt-get update

sudo apt-get install -y \
    gcc-14 \
    g++-14 \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libxxxf86vm-dev \
    libwayland-dev \
    libxkbcommon-dev
```

#### 2. Install Xmake

```bash
curl -fsSL https://xmake.io/shget.text | bash

source ~/.xmake/profile
```

#### 3. Clone the Repository

```bash
git clone --recursive https://github.com/TamasPetii/SynapseEngine.git

cd SynapseEngine
```

#### 4. Configure the Compiler

Ensure that **both vcpkg and SynapseEngine use GCC 14**.

```bash
export CC=gcc-14
export CXX=g++-14
```

#### 5. Bootstrap vcpkg

```bash
cd External/vcpkg

./bootstrap-vcpkg.sh

cd ../..
```

#### 6. Configure & Build

Configure the project using the GCC 14 toolchain and build it.

```bash
xmake f -p linux -a x64 -m release --cc=gcc-14 --cxx=g++-14 -y

xmake
```

> The **Visual Studio Code Xmake extension** works on Linux exactly the same way as on Windows, providing a seamless graphical workflow for configuring, building, and running the engine.

## Notes

* This is a research and architecture-focused engine
* Not intended as a plug-and-play game engine
* Designed to explore modern high-performance rendering techniques

## Licensing

SynapseEngine is dual-licensed:

- GNU Affero General Public License v3.0 (AGPLv3) for open-source and non-commercial use
- Commercial license available for proprietary/commercial usage

If you want to use SynapseEngine in a closed-source product, commercial game, proprietary engine, or commercial environment without AGPL obligations, you must obtain a commercial license.

For commercial licensing inquiries:

tamaspeti3451@gmail.com