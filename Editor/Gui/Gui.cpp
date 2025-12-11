#include "Gui.h"
#include <GLFW/glfw3.h>
#include "Editor/Gui/Windows/ComponentWindow.h"
#include "Editor/Gui/Windows/EntityWindow.h"
#include "Editor/Gui/Windows/GlobalSettingsWindow.h"
#include "Editor/Gui/Windows/BenchmarkWindow.h"
#include "Editor/Gui/Windows/ViewportWindow.h"
#include "Editor/Gui/Windows/GraphicsDebugWindow.h"
#include "Editor/Gui/Windows/FileSystemWindow.h"

Gui::Gui(GLFWwindow* window)
{
	Initialize(window);
}

Gui::~Gui()
{
	Cleanup();
}

void Gui::Initialize(GLFWwindow* window)
{
	auto instance = Vk::VulkanContext::GetContext()->GetInstance();
	auto physicalDevice = Vk::VulkanContext::GetContext()->GetPhysicalDevice();
	auto device = Vk::VulkanContext::GetContext()->GetDevice();
	auto swapChain = Vk::VulkanContext::GetContext()->GetSwapChain();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	io.Fonts->AddFontDefault();
	ImFontConfig config;
	config.MergeMode = true;
	config.GlyphOffset.y = 2.f;
	config.GlyphMinAdvanceX = 16.0f;
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	io.Fonts->AddFontFromFileTTF("../Assets/Fonts/fa-solid-900.ttf", 13.0f, &config, icon_ranges);

	ImGui_ImplGlfw_InitForVulkan(window, true);

	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
	poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
	poolInfo.pPoolSizes = poolSizes;

	VK_CHECK_MESSAGE(vkCreateDescriptorPool(device->Value(), &poolInfo, nullptr, &imguiPool), "Couldn't create imgui descriptor pool");

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance->Value();
	init_info.PhysicalDevice = physicalDevice->Value();
	init_info.Device = device->Value();
	init_info.QueueFamily = physicalDevice->GetQueueFamilyIndices().graphicsFamily.value();
	init_info.Queue = device->GetQueue(Vk::QueueType::GRAPHICS);
	init_info.DescriptorPool = imguiPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = static_cast<uint32_t>(swapChain->GetImages().size());
	init_info.ImageCount = static_cast<uint32_t>(swapChain->GetImages().size());
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.UseDynamicRendering = true;
	init_info.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapChain->GetImageFormat();

	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	SetStyle();

	windows["Viewport"] = std::make_shared<ViewportWindow>();
	windows["Entity"] = std::make_shared<EntityWindow>();
	windows["Component"] = std::make_shared<ComponentWindow>();
	windows["GlobalSettings"] = std::make_shared<GlobalSettingsWindow>();
	windows["BenchmarkWindow"] = std::make_shared<BenchmarkWindow>();
	windows["GraphicsDebugWIndow"] = std::make_shared<GraphicsDebugWindow>();

    windows["FileSystemWindow"] = std::make_shared<FileSystemWindow>(
        std::make_shared<NativeFileSystem>(),
        std::filesystem::absolute("../Assets").string()
    );
}

void Gui::Cleanup()
{
	auto device = Vk::VulkanContext::GetContext()->GetDevice();
	vkDeviceWaitIdle(device->Value());

	ImGui_ImplVulkan_Shutdown();
	vkDestroyDescriptorPool(device->Value(), imguiPool, nullptr);
}

void Gui::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
	for (VkDescriptorSet descriptorSet : imguiDescriptorSets[frameIndex])
		ImGui_ImplVulkan_RemoveTexture(descriptorSet);
	imguiDescriptorSets[frameIndex].clear();

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	ImGui::ShowDemoWindow();

	auto& textureSet = imguiDescriptorSets[frameIndex];
	windows["Viewport"]->Render(registry, resourceManager, textureSet, frameIndex);
	windows["Entity"]->Render(registry, resourceManager, textureSet, frameIndex);
	windows["Component"]->Render(registry, resourceManager, textureSet, frameIndex);
	windows["GlobalSettings"]->Render(registry, resourceManager, textureSet, frameIndex);
	windows["BenchmarkWindow"]->Render(registry, resourceManager, textureSet, frameIndex);
	windows["GraphicsDebugWIndow"]->Render(registry, resourceManager, textureSet, frameIndex);
    windows["FileSystemWindow"]->Render(registry, resourceManager, textureSet, frameIndex);

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Gui::SetStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Geometria és Méretek (Unreal-os, szögletesebb stílus) ---
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 5.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f); // Tömörebb táblázatok
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 14.0f; // Kicsit vastagabb, könnyebben fogható
    style.GrabMinSize = 10.0f;

    // --- Lekerekítések (Sharp / Flat Design) ---
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f; // A gomboknak nem kell keret, modern hatás
    style.TabBorderSize = 0.0f;

    style.WindowRounding = 2.0f; // Minimális kerekítés az ablakokon
    style.ChildRounding = 2.0f;
    style.FrameRounding = 3.0f; // A gombok/inputok épp csak lágyítva
    style.PopupRounding = 2.0f;
    style.ScrollbarRounding = 9.0f; // A scrollbar maradhat kerek
    style.GrabRounding = 2.0f;
    style.LogSliderDeadzone = 4.0f;
    style.TabRounding = 2.0f;

    // --- Színek (Deep Dark Grey / Slate Theme) ---

    // Alap szövegek
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f); // Törtfehér, szemkímélõbb
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Ablak hátterek (Nagyon sötét szürke)
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    // Keretek és elválasztók
    colors[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 0.80f); // Szinte fekete keret
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Input mezõk és Gombok alapja (Sötétebb, mint az ablak)
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.67f);

    // Címsorok (Title Bar)
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // Görgetõsáv
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

    // "Pipa" és Sliderek (Accent Color - Itt egy "Unreal Orange"-os beütés helyett egy "Tech Blue"-t használunk)
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Gombok
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // Semleges szürke
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f); // Aktív gomb kékül

    // Fejlécek (Pl. TreeView elemek)
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Elválasztók
    colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // Átméretezõ sarok
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

    // Fülek (Tabs) - Ez kritikus az UE lookhoz
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.11f, 0.11f, 0.86f);       // Inaktív fül beleolvad
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);       // Aktív fül kicsit világosabb
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);       // Kék csík a tetején (Modern ImGui feature)
    colors[ImGuiCol_TabDimmed] = ImVec4(0.08f, 0.08f, 0.08f, 0.97f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

    // Plotok és grafikonok
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

    // Navigáció
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}