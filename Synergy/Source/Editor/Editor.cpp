
#include "Snpch.h"
#include "Editor.h"
#include "API/VulkanContext.h"


namespace SNG
{

	struct EditorData
	{
		Window* Window = nullptr;
		std::vector<VkCommandBuffer> CommandBuffers;
		std::function<void()> MenuBarCallback = []() {};
	};

	static EditorData s_Data;




	static void SetTheme()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.15f, 0.18f, 1.0f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = colors[ImGuiCol_WindowBg];
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.25f, 0.31f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.32f, 0.35f, 0.41f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.45f, 0.51f, 1.0f);
		colors[ImGuiCol_Border] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_TitleBg] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = colors[ImGuiCol_WindowBg];
		colors[ImGuiCol_CheckMark] = ImVec4(0.52f, 0.55f, 0.61f, 1.0f);
		colors[ImGuiCol_ScrollbarBg] = colors[ImGuiCol_WindowBg];
		colors[ImGuiCol_ScrollbarGrab] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_ScrollbarGrabHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_ScrollbarGrabActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_SliderGrab] = colors[ImGuiCol_CheckMark];
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.62f, 0.65f, 0.71f, 1.0f);
		colors[ImGuiCol_Button] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_ButtonHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_ButtonActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_Header] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_HeaderHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_HeaderActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_Separator] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_SeparatorHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_SeparatorActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_ResizeGrip] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_ResizeGripHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_ResizeGripActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_Tab] = ImVec4(0, 0, 0, 0);
		colors[ImGuiCol_TabHovered] = colors[ImGuiCol_FrameBgHovered];
		colors[ImGuiCol_TabActive] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_TabUnfocused] = colors[ImGuiCol_Tab];
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_Tab];
		colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_FrameBgActive];
		colors[ImGuiCol_DockingEmptyBg] = colors[ImGuiCol_FrameBg];
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.6f, 0.6f, 0.6f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}



	void Editor::Initialize(Window* window)
	{
		s_Data.Window = window;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		float fontSize = 18.0f;
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/Fonts/NotoSans-Bold.ttf", fontSize);

		ImGui::StyleColorsDark();
		SetTheme();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForVulkan(s_Data.Window->Raw(), true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = VulkanContext::GetInstance();
		initInfo.Device = VulkanContext::GetDevice();
		initInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.MinImageCount = VulkanContext::GetSwapchainImageCount();
		initInfo.ImageCount = VulkanContext::GetSwapchainImageCount();
		initInfo.ColorAttachmentFormat = VulkanContext::GetSwapchainImageFormat();
		initInfo.DescriptorPool = VulkanContext::GetDescriptorPool();
		initInfo.Queue = VulkanContext::GetQueue();

		ImGui_ImplVulkan_Init(&initInfo, VulkanContext::GetRenderPass());
		ImGui_ImplVulkan_CreateFontsTexture();


		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanContext::GetCommandPool();
		allocInfo.commandBufferCount = VulkanContext::GetSwapchainImageCount();
		
		s_Data.CommandBuffers.resize(VulkanContext::GetSwapchainImageCount());
		SNG_CHECK_VULKAN_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo,
			s_Data.CommandBuffers.data()), "Failed to allocate a Vulkan command buffer");
	}

	void Editor::Shutdown()
	{
		VulkanContext::Wait();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Editor::SetMenuBarCallback(const std::function<void()>& func)
	{
		s_Data.MenuBarCallback = func;
	}

	void Editor::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			static bool dockspace_open = true;
			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				window_flags |= ImGuiWindowFlags_NoBackground;
			}
			else
			{
				dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Editor dockspace", &dockspace_open, window_flags);
			if (!opt_padding) ImGui::PopStyleVar();
			if (opt_fullscreen) ImGui::PopStyleVar(2);

			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			if (ImGui::BeginMenuBar())
			{
				s_Data.MenuBarCallback();
				ImGui::EndMenuBar();
			}
		}
	}

	void Editor::EndFrame()
	{
		ImGui::End(); // Dockspace

		ImGuiIO& io = ImGui::GetIO();
		Vec2 windowSize = s_Data.Window->GetSize();
		io.DisplaySize = ImVec2((float)windowSize.x, (float)windowSize.y);

		uint32_t currentFrame = VulkanContext::GetCurrentFrameIndex();
		uint32_t swapchainImageIndex = VulkanContext::GetSwapchainImageIndex();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		SNG_CHECK_VULKAN_RESULT(vkBeginCommandBuffer(s_Data.CommandBuffers[currentFrame],
			&beginInfo), "Failed to begin a Vulkan command buffer");

		VkClearValue clearValues[1] = {};
		clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = clearValues;
		renderPassInfo.renderPass = VulkanContext::GetRenderPass();
		renderPassInfo.framebuffer = VulkanContext::GetFramebuffers()[swapchainImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = VulkanContext::GetSwapchainImageExtent();
		vkCmdBeginRenderPass(s_Data.CommandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), s_Data.CommandBuffers[currentFrame]);
		vkCmdEndRenderPass(s_Data.CommandBuffers[currentFrame]);
		vkEndCommandBuffer(s_Data.CommandBuffers[currentFrame]);
		VulkanContext::PushCommandBuffer(s_Data.CommandBuffers[currentFrame], false);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}
}
