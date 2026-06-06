set_project("SynapseEngine")
set_version("1.0.0")

set_config("vcpkg", os.projectdir() .. "/../External/vcpkg")

set_allowedmodes("debug", "release", "dist", "performance")
add_rules("mode.debug", "mode.release")

set_languages("c17", "cxx23")
set_warnings("allextra")
set_policy("build.warning", true)

set_targetdir("Binaries/$(os)-$(arch)/$(mode)")
set_objectdir("Intermediates/$(os)-$(arch)/$(mode)/$(name)")

if is_plat("windows") then
    add_cxflags("/bigobj", "/GR-")
elseif is_plat("linux") then
    add_cxflags("-fno-rtti", "-std=c++23", {force = true})
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
    "JPH_OBJECT_STREAM",
    "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
    "NOMINMAX",
    "WIN32_LEAN_AND_MEAN",
    "_CRT_SECURE_NO_WARNINGS"
)

if is_mode("debug") then
    add_defines("SYN_DEBUG", "_DEBUG")
    if is_plat("windows") then set_runtimes("MDd") end
elseif is_mode("release") then
    add_defines("SYN_RELEASE", "NDEBUG")
    if is_plat("windows") then set_runtimes("MD") end
elseif is_mode("dist") then
    add_defines("SYN_DIST", "NDEBUG")
    if is_plat("windows") then set_runtimes("MD") end
    set_optimize("fastest")
    set_symbols("none")
    set_policy("build.optimization.lto", true)
elseif is_mode("performance") then
    add_defines("SYN_PERFORMANCE", "SYN_DIST", "NDEBUG")
    if is_plat("windows") then set_runtimes("MD") end
    set_optimize("fastest")
    set_symbols("none")
    set_policy("build.optimization.lto", true)
end

local imgui_name = "vcpkg::imgui[docking-experimental,glfw-binding,vulkan-binding]"

local imgui_cfg = {
    shared = false,
    debug = is_mode("debug"),
    runtimes = (is_plat("windows") and (is_mode("debug") and "MDd" or "MD")) or nil
}

add_requires(imgui_name, {configs = imgui_cfg})

local vcpkg_packages = {
    "vcpkg::glm",
    "vcpkg::glfw3",
    "vcpkg::vulkan-headers",
    "vcpkg::vulkan-memory-allocator",
    "vcpkg::shaderc",
    "vcpkg::glslang",
    "vcpkg::spirv-tools",
    "vcpkg::vulkan-loader",
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
    "vcpkg::tomlplusplus",
}

for _, pkg in ipairs(vcpkg_packages) do
    local cfg = {shared = false}
    if is_mode("debug") then
        cfg.debug = true
        if is_plat("windows") then cfg.runtimes = "MDd" end
    else
        if is_plat("windows") then cfg.runtimes = "MD" end
    end
    add_requires(pkg, {configs = cfg})
end

add_packages(imgui_name, table.unpack(vcpkg_packages))

target("Engine")
    set_kind("shared")
    add_files("Engine/**.cpp")
    add_headerfiles(
        "Engine/**.h",
        "Engine/**.hpp",
        "Engine/**.glsl",
        "Engine/**.vert",
        "Engine/**.frag",
        "Engine/**.comp",
        "Engine/**.geom",
        "Engine/**.mesh",
        "Engine/**.task",
        "Engine/**.json"
    )
    add_defines("SYN_BUILD_DLL", "VK_NO_PROTOTYPES")

target("EditorCore")
    set_kind("static")
    add_files("EditorCore/**.cpp")
    add_headerfiles("EditorCore/**.h", "EditorCore/**.hpp")
    add_deps("Engine")

target("Editor")
    set_kind("binary")

    if is_plat("windows") then
        add_syslinks("gdi32", "user32", "shell32")
    elseif is_plat("linux") then
        add_syslinks("pthread", "dl", "m")
    end

    add_files("Editor/**.cpp", "../External/ImGuiFileDialog/*.cpp", "../External/imgui-node-editor/*.cpp")
    add_headerfiles("Editor/**.h", "Editor/**.hpp")
    add_deps("Engine", "EditorCore")
    set_rundir("$(projectdir)")

target("UnitTests")
    set_kind("binary")
    add_files("UnitTests/**.cpp")
    add_headerfiles("UnitTests/**.h", "UnitTests/**.hpp")
    add_deps("Engine")