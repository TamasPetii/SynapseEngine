set_project("SynapseEngine")
set_version("1.0.0")

set_allowedmodes("debug", "release", "dist", "performance")
add_rules("mode.debug", "mode.release")

set_languages("c17", "cxx23")
set_warnings("allextra")
set_policy("build.warning", true)

set_targetdir("Binaries/$(os)-$(arch)/$(mode)")
set_objectdir("Intermediates/$(os)-$(arch)/$(mode)/$(name)")

if is_plat("windows") then
    add_cxflags("/bigobj")
    add_cxflags("/GR-") 
elseif is_plat("linux") then
    add_cxflags("-fno-rtti")
end

add_includedirs(
    ".",
    "../External/vulkan_radix_sort/include",
    "../External/ImGuiFileDialog",
    "../External/imgui-node-editor",
    "../External/IconFontCppHeaders"
)
add_defines(
    "_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING",
    "USE_STD_FILESYSTEM",
    "VK_ENABLE_BETA_EXTENSIONS",
    "SPIRV_REFLECT_USE_SYSTEM_SPIRV_H",
    "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    "VK_NO_PROTOTYPES",
    "JPH_OBJECT_STREAM",
    "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
    "NOMINMAX",
    "WIN32_LEAN_AND_MEAN",
    "_CRT_SECURE_NO_WARNINGS"
)

if is_mode("debug") then
    add_defines("SYN_DEBUG", "_DEBUG")
    set_runtimes("MDd")
elseif is_mode("release") then
    add_defines("SYN_RELEASE", "NDEBUG")
    set_runtimes("MD")
elseif is_mode("dist") then
    add_defines("SYN_DIST", "NDEBUG")
    set_runtimes("MD")
    set_optimize("fastest")
    set_symbols("none")
    set_policy("build.optimization.lto", true)
elseif is_mode("performance") then
    add_defines("SYN_PERFORMANCE", "SYN_DIST", "NDEBUG")
    set_runtimes("MD")
    set_optimize("fastest")
    set_symbols("none")
    set_policy("build.optimization.lto", true)
end

local vcpkg_packages = {
    "vcpkg::glm",
    "vcpkg::glfw3",
    "vcpkg::imgui[docking-experimental]",
    "vcpkg::vulkan-headers",
    "vcpkg::vulkan-memory-allocator",
    "vcpkg::shaderc",
    "vcpkg::volk",
    "vcpkg::assimp",
    "vcpkg::meshoptimizer",
    "vcpkg::stb",
    "vcpkg::nlohmann-json",
    "vcpkg::gli",
    "vcpkg::gtest",
    "vcpkg::spirv-reflect",
    "vcpkg::spirv-headers",
    "vcpkg::taskflow",
    "vcpkg::imguizmo",
    "vcpkg::joltphysics",
    "vcpkg::tinyxml2",
    "vcpkg::yaml-cpp",
    "vcpkg::tomlplusplus"
}

for _, pkg in ipairs(vcpkg_packages) do
    if is_plat("windows") then
        if is_mode("debug") then
            add_requires(pkg, {configs = {shared = false, debug = true, runtimes = "MDd"}})
        else
            add_requires(pkg, {configs = {shared = false, runtimes = "MD"}})
        end
    else
        if is_mode("debug") then
            add_requires(pkg, {configs = {shared = false, debug = true}})
        else
            add_requires(pkg, {configs = {shared = false}})
        end
    end
end

add_packages(table.unpack(vcpkg_packages))

target("Engine")
    set_kind("shared")
    add_files("Engine/**.cpp")
    add_headerfiles("Engine/**.h", "Engine/**.hpp")
    add_defines("SYN_BUILD_DLL")

target("EditorCore")
    set_kind("static")
    add_files("EditorCore/**.cpp")
    add_headerfiles("EditorCore/**.h", "EditorCore/**.hpp")
    add_deps("Engine")

target("Editor")
    set_kind("binary")
    add_files("Editor/**.cpp")
    add_headerfiles("Editor/**.h", "Editor/**.hpp")
    add_files("../External/ImGuiFileDialog/*.cpp")
    add_files("../External/imgui-node-editor/*.cpp")
    add_deps("Engine", "EditorCore")
    set_rundir("$(projectdir)")

target("UnitTests")
    set_kind("binary")
    add_files("UnitTests/**.cpp")
    add_headerfiles("UnitTests/**.h", "UnitTests/**.hpp")
    add_deps("Engine")