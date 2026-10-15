Forward & Forward+ Shader Directories - Naming & References:

[Forward+ Spatial Culling Techniques]
- DHAC [Dual-HiZ Adaptive Clustered - Synapse Engine]: Adaptive Z-sliced 3D clustering using O(1) Min/Max Dual-HiZ pyramid lookups for opaque and transparent geometry.
- DBAC [Depth-Bounded Adaptive Clustered - Synapse Engine]: Adaptive Z-sliced 3D clustering using per-tile raw depth-buffer min/max reduction (without Hi-Z).
- DBBC [Depth-Bounded Bitmask Clustered - Wicked Engine]: Single depth-bounded tile frustum combined with a 2.5D depth-slice bitmask light culling pass.
- DBC  [Depth-Bounded Clustered - Harada 2012]: Classic depth-bounded Forward+ building a single min/max depth frustum per screen-space tile.
- VFC  [View-Frustum Clustered - DOOM 2016]: Full camera view-frustum 3D grid partitioning in X/Y/Z without depth bounds.
- VFT  [View-Frustum Tiled - Traditional 2D Tiled]: Screen-space 2D tile partitioning in X/Y over the full camera depth range without Z-slicing.

[Traditional Forward Techniques (with GPU Frustum Light Culling)]
- DPF  [Depth-Prepass Forward]: Opaque depth prepass + DEPTH_EQUAL shading to eliminate lighting overdraw, evaluating only camera-frustum culled lights.
- VFF  [View-Frustum Forward]: Classic single-pass Forward shading without depth prepass, evaluating all camera-frustum culled lights per fragment.