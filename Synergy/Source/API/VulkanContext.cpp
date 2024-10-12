
#include "Snpch.h"
#include "VulkanContext.h"


namespace SNG
{

	struct VulkanContextData
	{
		const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		Window* Window = nullptr;
		VkInstance Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT Messenger = VK_NULL_HANDLE;
		VkSurfaceKHR Surface = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		uint32_t QueueFamilyIndex = (uint32_t)-1;
		VkDevice Device = VK_NULL_HANDLE;
		VkQueue Queue = VK_NULL_HANDLE;
		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
		std::vector<VkImage> SwapchainImages;
		std::vector<VkImageView> SwapchainImageViews;
		uint32_t SwapchainImageCount = (uint32_t)-1;
		VkFormat SwapchainImageFormat;
		VkExtent2D SwapchainImageExtent;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> Framebuffers;
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		VkCommandPool CommandPool = VK_NULL_HANDLE;
		std::vector<VkSemaphore> ComputeFinished;
		std::vector<VkSemaphore> ImagesAvailable;
		std::vector<VkSemaphore> RenderFinished;
		std::vector<VkFence> InFlight;
		std::vector<VkFence> ImagesInFlight;
		std::vector<VkFence> ComputeInFlight;
		uint32_t CurrentFrame = 0;
		uint32_t SwapchainImageIndex;

		std::vector<VkCommandBuffer> SubmitBuffers;
		std::vector<VkCommandBuffer> ComputeShaderCommands;
	};

	static VulkanContextData s_Data;



	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanErrorCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		SNG_ERROR("[VULKAN ERROR]: {0}", pCallbackData->pMessage);
		return VK_FALSE;
	}

	static VkBool32 CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func) return func(instance, pCreateInfo, pAllocator, pMessenger);
		else return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT messenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func) return func(instance, messenger, pAllocator);
		else SNG_ERROR("Can't find the function: 'vkDestroyDebugUtilsMessengerEXT'");
	}

	static void CreateSwapchain()
	{
		VkExtent2D extent;
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_Data.PhysicalDevice, s_Data.Surface, &capabilities);
		if (capabilities.currentExtent.width != UINT_MAX) extent = capabilities.currentExtent;
		else
		{
			auto windowSize = s_Data.Window->GetSize();
			extent = { (uint32_t)windowSize.x, (uint32_t)windowSize.y };
			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(s_Data.PhysicalDevice, s_Data.Surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(s_Data.PhysicalDevice, s_Data.Surface, &formatCount, formats.data());
		VkSurfaceFormatKHR format = formats[0];
		for (const auto& surfaceFormat : formats)
		{
			if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM && surfaceFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				format = surfaceFormat;
				break;
			}
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(s_Data.PhysicalDevice, s_Data.Surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(s_Data.PhysicalDevice, s_Data.Surface, &presentModeCount, presentModes.data());
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& surfacePresentMode : presentModes)
		{
			if (surfacePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				presentMode = surfacePresentMode;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = s_Data.Surface;
		swapchainCreateInfo.imageFormat = format.format;
		swapchainCreateInfo.imageColorSpace = format.colorSpace;
		swapchainCreateInfo.imageExtent = extent;
		swapchainCreateInfo.presentMode = presentMode;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.preTransform = capabilities.currentTransform;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;

		SNG_CHECK_VULKAN_RESULT(vkCreateSwapchainKHR(s_Data.Device, &swapchainCreateInfo,
			nullptr, &s_Data.Swapchain), "Failed to create a Vulkan swapchain");

		vkGetSwapchainImagesKHR(s_Data.Device, s_Data.Swapchain, &s_Data.SwapchainImageCount, nullptr);
		s_Data.SwapchainImages.resize(s_Data.SwapchainImageCount);
		vkGetSwapchainImagesKHR(s_Data.Device, s_Data.Swapchain, &s_Data.SwapchainImageCount, s_Data.SwapchainImages.data());

		s_Data.SwapchainImageViews.resize(s_Data.SwapchainImageCount);
		for (uint32_t i = 0; i < s_Data.SwapchainImageCount; i++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.format = format.format;
			viewInfo.image = s_Data.SwapchainImages[i];

			SNG_CHECK_VULKAN_RESULT(vkCreateImageView(s_Data.Device, &viewInfo,
				nullptr, &s_Data.SwapchainImageViews[i]), "Failed to create a Vulkan image view");
		}

		s_Data.SwapchainImageFormat = format.format;
		s_Data.SwapchainImageExtent = extent;
	}

	static void DestroySwapchain()
	{
		for (auto imageView : s_Data.SwapchainImageViews)
			vkDestroyImageView(s_Data.Device, imageView, nullptr);
		vkDestroySwapchainKHR(s_Data.Device, s_Data.Swapchain, nullptr);
	}

	static void CreateRenderPass()
	{
		VkAttachmentDescription attachments[1] = {};
		attachments[0].format = s_Data.SwapchainImageFormat;
		attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference references[1] = {};
		references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		references[0].attachment = 0;

		VkSubpassDescription subpasses[1] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = references;

		VkSubpassDependency dependencies[1] = {};
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = attachments;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = subpasses;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = dependencies;

		SNG_CHECK_VULKAN_RESULT(vkCreateRenderPass(s_Data.Device, &renderPassCreateInfo,
			nullptr, &s_Data.RenderPass), "Failed to create a Vulkan render pass");
	}

	static void DestroyRenderPass()
	{
		vkDestroyRenderPass(s_Data.Device, s_Data.RenderPass, nullptr);
	}

	static void CreateFramebuffers()
	{
		s_Data.Framebuffers.resize(s_Data.SwapchainImageCount);
		for (uint32_t i = 0; i < s_Data.SwapchainImageCount; i++)
		{
			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.renderPass = s_Data.RenderPass;
			framebufferCreateInfo.attachmentCount = 1;
			framebufferCreateInfo.pAttachments = &s_Data.SwapchainImageViews[i];
			framebufferCreateInfo.width = s_Data.SwapchainImageExtent.width;
			framebufferCreateInfo.height = s_Data.SwapchainImageExtent.height;
			framebufferCreateInfo.layers = 1;

			SNG_CHECK_VULKAN_RESULT(vkCreateFramebuffer(s_Data.Device, &framebufferCreateInfo,
				nullptr, &s_Data.Framebuffers[i]), "Failed to create a Vulkan framebuffer");
		}
	}

	static void DestroyFramebuffers()
	{
		for (auto framebuffer : s_Data.Framebuffers)
			vkDestroyFramebuffer(s_Data.Device, framebuffer, nullptr);
	}




	void VulkanContext::Initialize(Window* window)
	{
		SNG_TRACE("Vulkan initialization...");
		s_Data.Window = window;

		/* Instance and error callback */
		{
			VkApplicationInfo appInfo{};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Ray Tracer";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "Synergy Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			#if VULKAN_VALIDATION_LAYERS
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			#endif

			VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
			messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
			messengerCreateInfo.pfnUserCallback = VulkanErrorCallback;

			VkInstanceCreateInfo instanceInfo{};
			instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceInfo.pApplicationInfo = &appInfo;
			instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
			instanceInfo.ppEnabledExtensionNames = extensions.data();

			#if VULKAN_VALIDATION_LAYERS
				instanceInfo.enabledLayerCount = (uint32_t)s_Data.ValidationLayers.size();
				instanceInfo.ppEnabledLayerNames = s_Data.ValidationLayers.data();
				instanceInfo.pNext = &messengerCreateInfo;
			#else
				instanceInfo.enabledLayerCount = 0;
				instanceInfo.ppEnabledLayerNames = nullptr;
				instanceInfo.pNext = nullptr;
			#endif

			SNG_CHECK_VULKAN_RESULT(vkCreateInstance(&instanceInfo, nullptr, &s_Data.Instance), "Failed to create a Vulkan instance");

			#if VULKAN_VALIDATION_LAYERS
				SNG_CHECK_VULKAN_RESULT(CreateDebugUtilsMessengerEXT(s_Data.Instance, &messengerCreateInfo,
					nullptr, &s_Data.Messenger), "Failed to create a Vulkan messenger");
			#endif
		}

		/* Physical device and surface */
		{
			SNG_CHECK_VULKAN_RESULT(glfwCreateWindowSurface(s_Data.Instance, s_Data.Window->Raw(),
				nullptr, &s_Data.Surface), "Failed to create a Vulkan surface");

			uint32_t gpuCount = 0;
			vkEnumeratePhysicalDevices(s_Data.Instance, &gpuCount, nullptr);
			SNG_ASSERT(gpuCount != 0);
			std::vector<VkPhysicalDevice> gpus(gpuCount);
			vkEnumeratePhysicalDevices(s_Data.Instance, &gpuCount, gpus.data());

			s_Data.PhysicalDevice = gpus[0];
			for (uint32_t i = 0; i < gpuCount; i++)
			{
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(gpus[i], &props);

				if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					s_Data.PhysicalDevice = gpus[i];
					break;
				}
			}
		}

		/* Logical device and queue */
		{
			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(s_Data.PhysicalDevice, &queueCount, nullptr);
			std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(s_Data.PhysicalDevice, &queueCount, queueFamilies.data());

			for (uint32_t i = 0; i < queueCount; i++)
			{
				if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					s_Data.QueueFamilyIndex = i;
					break;
				}
			}

			SNG_ASSERT(s_Data.QueueFamilyIndex != (uint32_t)-1);

			float queuePriority = 1.0f;
			VkDeviceQueueCreateInfo queueCreateInfos[1] = {};
			queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfos[0].queueFamilyIndex = s_Data.QueueFamilyIndex;
			queueCreateInfos[0].pQueuePriorities = &queuePriority;
			queueCreateInfos[0].queueCount = 1;

			VkDeviceCreateInfo deviceCreateInfo{};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.enabledExtensionCount = (uint32_t)s_Data.DeviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = s_Data.DeviceExtensions.data();
			deviceCreateInfo.queueCreateInfoCount = 1;
			deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
			
			#if VULKAN_VALIDATION_LAYERS
				deviceCreateInfo.enabledLayerCount = (uint32_t)s_Data.ValidationLayers.size();
				deviceCreateInfo.ppEnabledLayerNames = s_Data.ValidationLayers.data();
			#endif

			SNG_CHECK_VULKAN_RESULT(vkCreateDevice(s_Data.PhysicalDevice, &deviceCreateInfo,
				nullptr, &s_Data.Device), "Failed to create a Vulkan device");

			vkGetDeviceQueue(s_Data.Device, s_Data.QueueFamilyIndex, 0, &s_Data.Queue);
		}

		/* Swapchain, images and views */
		{
			CreateSwapchain();
		}

		/* Render pass */
		{
			CreateRenderPass();
		}

		/* Framebuffers */
		{
			CreateFramebuffers();
		}

		/* Descriptor Pool */
		{
			VkDescriptorPoolSize poolSizes[] = {
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

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
			poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
			poolInfo.pPoolSizes = poolSizes;

			SNG_CHECK_VULKAN_RESULT(vkCreateDescriptorPool(s_Data.Device, &poolInfo,
				nullptr, &s_Data.DescriptorPool), "Failed to create a Vulkan descriptor pool");
		}

		/* Command Pool */
		{
			VkCommandPoolCreateInfo poolCreateInfo{};
			poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolCreateInfo.queueFamilyIndex = s_Data.QueueFamilyIndex;

			SNG_CHECK_VULKAN_RESULT(vkCreateCommandPool(s_Data.Device, &poolCreateInfo,
				nullptr, &s_Data.CommandPool), "Failed to create a Vulkan command pool");
		}

		/* Semaphores and fences */
		{
			VkSemaphoreCreateInfo semaphoreCreateInfo{};
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			s_Data.ImagesAvailable.resize(s_Data.SwapchainImageCount);
			s_Data.RenderFinished.resize(s_Data.SwapchainImageCount);
			s_Data.ComputeFinished.resize(s_Data.SwapchainImageCount);
			s_Data.InFlight.resize(s_Data.SwapchainImageCount);
			s_Data.ComputeInFlight.resize(s_Data.SwapchainImageCount);
			s_Data.ImagesInFlight.resize(s_Data.SwapchainImageCount, VK_NULL_HANDLE);

			for (uint32_t i = 0; i < s_Data.SwapchainImageCount; i++)
			{
				vkCreateSemaphore(s_Data.Device, &semaphoreCreateInfo, nullptr, &s_Data.ImagesAvailable[i]);
				vkCreateSemaphore(s_Data.Device, &semaphoreCreateInfo, nullptr, &s_Data.RenderFinished[i]);
				vkCreateSemaphore(s_Data.Device, &semaphoreCreateInfo, nullptr, &s_Data.ComputeFinished[i]);
				vkCreateFence(s_Data.Device, &fenceCreateInfo, nullptr, &s_Data.InFlight[i]);
				vkCreateFence(s_Data.Device, &fenceCreateInfo, nullptr, &s_Data.ComputeInFlight[i]);
			}
		}
	}

	void VulkanContext::Shutdown()
	{
		for (uint32_t i = 0; i < s_Data.SwapchainImageCount; i++)
		{
			vkDestroySemaphore(s_Data.Device, s_Data.ImagesAvailable[i], nullptr);
			vkDestroySemaphore(s_Data.Device, s_Data.RenderFinished[i], nullptr);
			vkDestroySemaphore(s_Data.Device, s_Data.ComputeFinished[i], nullptr);
			vkDestroyFence(s_Data.Device, s_Data.InFlight[i], nullptr);
			vkDestroyFence(s_Data.Device, s_Data.ComputeInFlight[i], nullptr);
		}

		vkDestroyCommandPool(s_Data.Device, s_Data.CommandPool, nullptr);
		vkDestroyDescriptorPool(s_Data.Device, s_Data.DescriptorPool, nullptr);

		DestroyFramebuffers();
		DestroyRenderPass();
		DestroySwapchain();

		vkDestroySurfaceKHR(s_Data.Instance, s_Data.Surface, nullptr);
		vkDestroyDevice(s_Data.Device, nullptr);

		#if VULKAN_VALIDATION_LAYERS
			DestroyDebugUtilsMessengerEXT(s_Data.Instance, s_Data.Messenger, nullptr);
		#endif

		vkDestroyInstance(s_Data.Instance, nullptr);
	}

	void VulkanContext::RecreateSwapchain()
	{
		DestroyFramebuffers();
		DestroyRenderPass();
		DestroySwapchain();
		CreateSwapchain();
		CreateRenderPass();
		CreateFramebuffers();
	}

	VkCommandBuffer VulkanContext::CreateCommandBuffer(bool begin)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = s_Data.CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		SNG_CHECK_VULKAN_RESULT(vkAllocateCommandBuffers(s_Data.Device, &allocInfo,
			&commandBuffer), "Failed to allocate a Vulkan command buffer");

		if (begin)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			SNG_CHECK_VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer,
				&beginInfo), "Failed to begin a Vulkan command buffer");
		}

		return commandBuffer;
	}

	void VulkanContext::PushCommandBuffer(VkCommandBuffer commandBuffer, bool end)
	{
		if (end) vkEndCommandBuffer(commandBuffer);
		s_Data.SubmitBuffers.push_back(commandBuffer);
	}

	void VulkanContext::SubmitComputeShader(VkCommandBuffer commandBuffer)
	{
		s_Data.ComputeShaderCommands.push_back(commandBuffer);
	}

	void VulkanContext::BeginFrame()
	{
		vkWaitForFences(s_Data.Device, 1, &s_Data.InFlight[s_Data.CurrentFrame], VK_TRUE, UINT64_MAX);
		vkAcquireNextImageKHR(s_Data.Device, s_Data.Swapchain, UINT64_MAX,
			s_Data.ImagesAvailable[s_Data.CurrentFrame], VK_NULL_HANDLE, &s_Data.SwapchainImageIndex);
	}

	void VulkanContext::EndFrame()
	{
		//===== COMPUTES
		if (!s_Data.ComputeShaderCommands.empty())
		{
			vkWaitForFences(VulkanContext::GetDevice(), 1, &s_Data.ComputeInFlight[s_Data.CurrentFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(VulkanContext::GetDevice(), 1, &s_Data.ComputeInFlight[s_Data.CurrentFrame]);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = (uint32_t)s_Data.ComputeShaderCommands.size();
			submitInfo.pCommandBuffers = s_Data.ComputeShaderCommands.data();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &s_Data.ComputeFinished[s_Data.CurrentFrame];

			SNG_CHECK_VULKAN_RESULT(vkQueueSubmit(s_Data.Queue, 1, &submitInfo,
				s_Data.ComputeInFlight[s_Data.CurrentFrame]), "Failed to submit Vulkan command buffers");
		}

		//===== GRAPHICS
		if (s_Data.ImagesInFlight[s_Data.SwapchainImageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(s_Data.Device, 1, &s_Data.ImagesInFlight[s_Data.SwapchainImageIndex], VK_TRUE, UINT64_MAX);
		s_Data.ImagesInFlight[s_Data.SwapchainImageIndex] = s_Data.InFlight[s_Data.CurrentFrame];

		std::vector<VkSemaphore> waitSemaphores = { s_Data.ImagesAvailable[s_Data.CurrentFrame] };
		if (!s_Data.ComputeShaderCommands.empty()) waitSemaphores.push_back(s_Data.ComputeFinished[s_Data.CurrentFrame]);

		std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		if (!s_Data.ComputeShaderCommands.empty()) waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = (uint32_t)s_Data.SubmitBuffers.size();
		submitInfo.pCommandBuffers = s_Data.SubmitBuffers.data();
		submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &s_Data.RenderFinished[s_Data.CurrentFrame];

		vkResetFences(s_Data.Device, 1, &s_Data.InFlight[s_Data.CurrentFrame]);
		SNG_CHECK_VULKAN_RESULT(vkQueueSubmit(s_Data.Queue, 1, &submitInfo,
			s_Data.InFlight[s_Data.CurrentFrame]), "Failed to submit Vulkan command buffers");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &s_Data.Swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &s_Data.RenderFinished[s_Data.CurrentFrame];
		presentInfo.pImageIndices = &s_Data.SwapchainImageIndex;

		SNG_CHECK_VULKAN_RESULT(vkQueuePresentKHR(s_Data.Queue, &presentInfo), "Failed to present an image");
		s_Data.CurrentFrame = (s_Data.CurrentFrame + 1) % s_Data.SwapchainImageCount;
		s_Data.SubmitBuffers.clear();
		s_Data.ComputeShaderCommands.clear();
	}



	void VulkanContext::Wait() { vkDeviceWaitIdle(s_Data.Device); }
	VkInstance VulkanContext::GetInstance() { return s_Data.Instance; }
	VkDevice VulkanContext::GetDevice() { return s_Data.Device; }
	VkPhysicalDevice VulkanContext::GetPhysicalDevice() { return s_Data.PhysicalDevice; }
	uint32_t VulkanContext::GetSwapchainImageCount() { return s_Data.SwapchainImageCount; }
	VkFormat VulkanContext::GetSwapchainImageFormat() { return s_Data.SwapchainImageFormat; }
	VkExtent2D VulkanContext::GetSwapchainImageExtent() { return s_Data.SwapchainImageExtent; }
	VkDescriptorPool VulkanContext::GetDescriptorPool() { return s_Data.DescriptorPool; }
	VkQueue VulkanContext::GetQueue() { return s_Data.Queue; }
	VkRenderPass VulkanContext::GetRenderPass() { return s_Data.RenderPass; }
	VkCommandPool VulkanContext::GetCommandPool() { return s_Data.CommandPool; }
	uint32_t VulkanContext::GetCurrentFrameIndex() { return s_Data.CurrentFrame; }
	uint32_t VulkanContext::GetSwapchainImageIndex() { return s_Data.SwapchainImageIndex; }
	const std::vector<VkFramebuffer>& VulkanContext::GetFramebuffers() { return s_Data.Framebuffers; }
}
