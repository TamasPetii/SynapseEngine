# Synapse Engine - Development Journey & Architecture

## Dependency Management

We utilize **vcpkg** in **Manifest Mode** to handle third-party libraries (GLFW, GLM, etc.) strictly and efficiently.

- **Submodule Strategy:** `vcpkg` is added as a git submodule to ensure every developer uses the exact same package manager version.
- **Bootstrapping:** The environment is initialized by running the `bootstrap-vcpkg.bat` script, which builds the `vcpkg.exe` executable locally.
- **Manifest Mode:** Dependencies are defined in `vcpkg.json`. When building the project, vcpkg automatically reads this manifest.
- **Localized Artifacts:** All downloaded headers, libs, and DLLs are stored in the `vcpkg_installed` folder at the Solution root, keeping the global system clean.

## Build System & Configuration

To maintain consistency across multiple projects (Engine, Editor, Core), we use a centralized **`Directory.Build.props`** file.

- **Centralized Settings:** Defines the C++ Language Standard (e.g., C++23) and enables vcpkg integration globally.
- **Output Paths:** Configures clean output directories to separate source code from build artifacts:
  - **`Binaries/`**: Contains the final executables and DLLs (e.g., `Binaries/x64/Debug/`).
  - **`Intermediates/`**: Contains temporary object files (`.obj`), speeding up re-compilation.

## Project Architecture

The solution is split into three distinct layers to enforce separation of concerns:

1. **Engine (`.dll`)**
   - The core runtime library. Contains the low-level systems (Vulkan Renderer, Scene Management, etc...).
   
2. **EditorCore (`.lib`)**
   - Acts as the "ViewModel" and logic layer for the tools.
   - Handles UI abstractions (e.g., `TransformComponentUI`), Command pattern implementation (Undo/Redo), and Editor-specific logic.

3. **Editor (`.exe`)**
   - The application entry point.
   - Responsible for Window creation, ImGui context setup, ImGui abstractions and UI windows, and driving the application loop.

## Build Configurations

We support three specific build targets to cover all stages of development:

- **Debug:** No optimizations, full debug symbols, and verbose logging. Best for step-by-step debugging.
- **Release:** Optimized for performance (O2), but retains some logging and assertions for development testing.
- **Dist (Distribution):** Maximum optimization, all debug symbols stripped, console disabled. This is the final version shipped to the user.

## CI/CD Pipeline

We utilize **GitHub Actions** to ensure code stability and automate the deployment process efficiently.

- **Clean Room Testing:** Every push and pull request triggers a full build on a clean `windows-latest` virtual machine. This eliminates "it works on my machine" issues by verifying dependencies and paths in a fresh environment.
- **Smart Caching:** To significantly reduce build times, the pipeline caches `vcpkg` binary artifacts. Third-party libraries are only rebuilt if `vcpkg.json` is modified.
- **Automated Distribution:** Upon a successful build, the system compiles the project using the **Dist** configuration (optimized, stripped symbols) and automatically uploads the resulting binaries as a downloadable artifact.