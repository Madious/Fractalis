
#pragma once
#include "Core/Window.h"


namespace SNG
{

	class VulkanContext
	{
	public:

		static void Initialize(Window* window);
		static void Shutdown();
		static void RecreateSwapchain();

		static VkCommandBuffer CreateCommandBuffer(bool begin = true);
		static void PushCommandBuffer(VkCommandBuffer commandBuffer, bool end = true);
		static void SubmitComputeShader(VkCommandBuffer commandBuffer);
		static void BeginFrame();
		static void EndFrame();

		static void Wait();
		static VkInstance GetInstance();
		static VkDevice GetDevice();
		static VkPhysicalDevice GetPhysicalDevice();
		static uint32_t GetSwapchainImageCount();
		static VkFormat GetSwapchainImageFormat();
		static VkExtent2D GetSwapchainImageExtent();
		static VkDescriptorPool GetDescriptorPool();
		static VkQueue GetQueue();
		static VkRenderPass GetRenderPass();
		static VkCommandPool GetCommandPool();
		static uint32_t GetCurrentFrameIndex();
		static uint32_t GetSwapchainImageIndex();
		static const std::vector<VkFramebuffer>& GetFramebuffers();
	};
}
